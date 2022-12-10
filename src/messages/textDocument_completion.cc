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

  std::string p