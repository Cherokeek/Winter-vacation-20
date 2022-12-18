// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "fuzzy_match.hh"
#include "include_complete.hh"
#include "log.hh"
#include "message_handler.hh"
#include "pipeline.hh"
#include "sema_manager.hh"
#include "working_files.hh"

#include <clang/Sema/CodeCompleteConsumer.h>
#include <clang/Sema/Sema.h>
#include <llvm/ADT/Twine.h>

#if LLVM_VERSION_MAJOR < 8
#include <regex>
#endif

namespace ccls {
using namespace clang;
using namespace llvm;

REFLECT_UNDERLYING(InsertTextFormat);
REFLECT_UNDERLYING(CompletionItemKind);

void reflect(JsonWriter &vis, CompletionItem &v) {
  reflectMemberStart(vis);
  REFLECT_MEMBER(label);
  REFLECT_MEMBER(kind);
  REFLECT_MEMBER(detail);
  if (v.documentation.size())
    REFLECT_MEMBER(documentation);
  REFLECT_MEMBER(sortText);
  if (v.filterText.size())
    REFLECT_MEMBER(filterText);
  REFLECT_MEMBER(insertTextFormat);
  REFLECT_MEMBER(textEdit);
  if (v.additionalTextEdits.size())
    REFLECT_MEMBER(additionalTextEdits);
  reflectMemberEnd(vis);
}

namespace {
struct CompletionList {
  bool isIncomplete = false;
  std::vector<CompletionItem> items;
};
REFLECT_STRUCT(CompletionList, isIncomplete, items);

#if LLVM_VERSION_MAJOR < 8
void decorateIncludePaths(const std::smatch &match,
                          std::vector<CompletionItem> *items, char quote) {
  std::string spaces_after_include = " ";
  if (match[3].compare("include") == 0 && quote != '\0')
    spaces_after_include = match[4].str();

  std::string prefix =
      match[1].str() + '#' + match[2].str() + "include" + spaces_after_include;
  std::string suffix = match[7].str();

  for (CompletionItem &item : *items) {
    char quote0, quote1;
    if (quote != '"')
      quote0 = '<', quote1 = '>';
    else
      quote0 = quote1 = '"';

    item.textEdit.newText =
        prefix + quote0 + item.textEdit.newText + quote1 + suffix;
    item.label = prefix + quote0 + item.label + quote1 + suffix;
  }
}

struct ParseIncludeLineResult {
  bool ok;
  std::string keyword;
  std::string quote;
  std::string pattern;
  std::smatch match;
};

ParseIncludeLineResult ParseIncludeLine(const std::string &line) {
  static const std::regex pattern("(\\s*)"       // [1]: spaces before '#'
                                  "#"            //
                                  "(\\s*)"       // [2]: spaces after '#'
                                  "([^\\s\"<]*)" // [3]: "include"
                                  "(\\s*)"       // [4]: spaces before quote
                                  "([\"<])?"     // [5]: the first quote char
                                  "([^\\s\">]*)" // [6]: path of file
                                  "[\">]?"       //
                                  "(.*)");       // [7]: suffix after quote char
  std::smatch match;
  bool ok = std::regex_match(line, match, pattern);
  return {ok, match[3], match[5], match[6], match};
}
#endif

// Pre-filters completion responses before sending to vscode. This results in a
// significantly snappier completion experience as vscode is easily overloaded
// when given 1000+ completion items.
void filterCandidates(CompletionList &result, const std::string &complete_text,
                      Position begin_pos, Position end_pos,
                      const std::string &buffer_line) {
  assert(begin_pos.line == end_pos.line);
  auto &items = result.items;

  // People usually does not want to insert snippets or parenthesis when
  // changing function or type names, e.g. "str.|()" or "std::|<int>".
  bool has_open_paren = false;
  for (int c = end_pos.character; c < buffer_line.size(); ++c) {
    if (buffer_line[c] == '(' || buffer_line[c] == '<')
      has_open_paren = true;
    if (!isspace(buffer_line[c]))
      break;
  }

  auto finalize = [&]() {
    int max_num = g_config->completion.maxNum;
    if (items.size() > max_num) {
      items.resize(max_num);
      result.isIncomplete = true;
    }

    int overwrite_len = 0;
    for (auto &item : items) {
      auto &edits = item.additionalTextEdits;
      if (edits.size() && edits[0].range.end == begin_pos) {
        Position start = edits[0].range.start, end = edits[0].range.end;
        if (start.line == begin_pos.line) {
          overwrite_len =
              std::max(overwrite_len, end.character - start.character);
        } else {
          overwrite_len = -1;
          break;
        }
      }
    }

    Position overwrite_begin = {begin_pos.line,
                                begin_pos.character - overwrite_len};
    std::string sort(4, ' ');
    for (auto &item : items) {
      item.textEdit.range = lsRange{begin_pos, end_pos};
      if (has_open_paren)
        item.textEdit.newText = item.filterText;
      // https://github.com/Microsoft/language-server-protocol/issues/543
      // Order of textEdit and additionalTextEdits is unspecified.
      auto &edits = item.additionalTextEdits;
      if (overwrite_len > 0) {
        item.textEdit.range.start = overwrite_begin;
        std::string orig =
            buffer_line.substr(overwrite_begin.character, overwrite_len);
        if (edits.size() && edits[0].range.end == begin_pos &&
            edits[0].range.start.line == begin_pos.line) {
          int cur_edit_len =
              edits[0].range.end.character - edits[0].range.start.character;
          item.textEdit.newText =
              buffer_line.substr(overwrite_begin.character,
                                 overwrite_len - cur_edit_len) +
              edits[0].newText + item.textEdit.newText;
          edits.erase(edits.begin());
        } else {
          item.textEdit.newText = orig + item.textEdit.newText;
        }
        item.filterText = orig + item.filterText;
      }
      if (item.filterText == item.label)
        item.filterText.clear();
      for (auto i = sort.size(); i && ++sort[i - 1] == 'A';)
        sort[--i] = ' ';
      item.sortText = sort;
    }
  };

  if (!g_config->completion.filterAndSort) {
    finalize();
    return;
  }

  if (complete_text.size()) {
    // Fuzzy match and remove awful candidates.
    bool sensitive = g_config->completion.caseSensitivity;
    FuzzyMatcher fuzzy(complete_text, sensitive);
    for (CompletionItem &item : items) {
      const std::string &filter =
          item.filterText.size() ? item.filterText : item.label;
      item.score_ = reverseSubseqMatch(complete_text, filter, sensitive) >= 0
                        ? fuzzy.match(filter, true)
                        : FuzzyMatcher::kMinScore;
    }
    items.erase(std::remove_if(items.begin(), items.end(),
                               [](const CompletionItem &item) {
                                 return item.score_ <= FuzzyMatcher::kMinScore;
                               }),
                items.end());
  }
  std::sort(items.begin(), items.end(),
            [](const CompletionItem &lhs, const CompletionItem &rhs) {
              int t = int(lhs.additionalTextEdits.size() -
                          rhs.additionalTextEdits.size());
              if (t)
                return t < 0;
              if (lhs.score_ != rhs.score_)
                return lhs.score_ > rhs.score_;
              if (lhs.priority_ != rhs.priority_)
                return lhs.priority_ < rhs.priority_;
              t = lhs.textEdit.newText.compare(rhs.textEdit.newText);
              if (t)
                return t < 0;
              t = lhs.label.compare(rhs.label);
              if (t)
                return t < 0;
              return lhs.filterText < rhs.filterText;
            });

  // Trim result.
  finalize();
}

CompletionItemKind getCompletionKind(CodeCompletionContext::Kind k,
                                     const CodeCompletionResult &r) {
  switch (r.Kind) {
  case CodeCompletionResult::RK_Declaration: {
    const Decl *d = r.Declaration;
    switch (d->getKind()) {
    case Decl::LinkageSpec:
      return CompletionItemKind::Keyword;
    case Decl::Namespace:
    case Decl::NamespaceAlias:
      return CompletionItemKind::Module;
    case Decl::ObjCCategory:
    case Decl::ObjCCategoryImpl:
    case Decl::ObjCImplementation:
    case Decl::ObjCInterface:
    case Decl::ObjCProtocol:
      return CompletionItemKind::Interface;
    case Decl::ObjCMethod:
      return CompletionItemKind::Method;
    case Decl::ObjCProperty:
      return CompletionItemKind::Property;
    case Decl::ClassTemplate:
      return CompletionItemKind::Class;
    case Decl::FunctionTemplate:
      return CompletionItemKind::Function;
    case Decl::TypeAliasTemplate:
      return CompletionItemKind::Class;
    case Decl::VarTemplate:
      if (cast<VarTemplateDecl>(d)->getTemplatedDecl()->isConstexpr())
        return CompletionItemKind::Constant;
      return CompletionItemKind::Variable;
    case Decl::TemplateTemplateParm:
      return CompletionItemKind::TypeParameter;
    case Decl::Enum:
      return CompletionItemKind::Enum;
    case Decl::CXXRecord:
    case Decl::Record:
      if (auto *rd = dyn_cast<RecordDecl>(d))
        if (rd->getTagKind() == TTK_Struct)
          return CompletionItemKind::Struct;
      return CompletionItemKind::Class;
    case Decl::TemplateTypeParm:
    case Decl::TypeAlias:
    case Decl::Typedef:
      return CompletionItemKind::TypeParameter;
    case Decl::Using:
    case Decl::ConstructorUsingShadow:
      return CompletionItemKind::Keyword;
    case Decl::Binding:
      return CompletionItemKind::Variable;
    case Decl::Field:
    case Decl::ObjCIvar:
      return CompletionItemKind::Field;
    case Decl::Function:
      return CompletionItemKind::Function;
    case Decl::CXXMethod:
      return CompletionItemKind::Method;
    case Decl::CXXConstructor:
      return CompletionItemKind::Constructor;
    case Decl::CXXConversion:
    case Decl::CXXDestructor:
      return CompletionItemKind::Method;
    case Decl::Var:
    case Decl::Decomposition:
    case Decl::ImplicitParam:
    case Decl::ParmVar:
    case Decl::VarTemplateSpecialization:
    case Decl::VarTemplatePartialSpecialization:
      if (cast<VarDecl>(d)->isConstexpr())
        return CompletionItemKind::Constant;
      return CompletionItemKind::Variable;
    case Decl::EnumConstant:
      return CompletionItemKind::EnumMember;
    case Decl::IndirectField:
      return CompletionItemKind::Field;

    default:
      LOG_S(WARNING) << "Unhandled " << int(d->getKind());
      return CompletionItemKind::Text;
    }
    break;
  }
  case CodeCompletionResult::RK_Keyword:
    return CompletionItemKind::Keyword;
  case CodeCompletionResult::RK_Macro:
    return CompletionItemKind::Reference;
  case CodeCompletionResult::RK_Pattern:
#if LLVM_VERSION_MAJOR >= 8
    if (k == CodeCompletionContext::CCC_IncludedFile)
      return CompletionItemKind::File;
#endif
    return CompletionItemKind::Snippet;
  }
}

void buildItem(const CodeCompletionResult &r, const CodeCompletionString &ccs,
               std::vector<CompletionItem> &out) {
  assert(!out.empty());
  auto first = out.size() - 1;
  bool ignore = false;
  std::string result_type;

  for (const auto &chunk : ccs) {
    CodeCompletionString::ChunkKind kind = chunk.Kind;
    std::string text;
    switch (kind) {
    case CodeCompletionString::CK_TypedText:
      text = chunk.Text;
      for (auto i = first; i < out.size(); i++)
        out[i].filterText = text;
      break;
    case CodeCompletionString::CK_Placeholder:
      text = chunk.Text;
      for (auto i = first; i < out.size(); i++)
        out[i].parameters_.push_back(text);
      break;
    case CodeCompletionString::CK_Informative:
      if (StringRef(chunk.Text).endswith("::"))
        continue;
      text = chunk.Text;
      break;
    case CodeCompletionString::CK_ResultType:
   