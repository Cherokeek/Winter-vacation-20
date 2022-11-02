// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "lsp.hh"
#include "query.hh"

#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace ccls {
struct SemaManager;
struct VFS;
struct IncludeComplete;
struct Project;
struct WorkingFile;
struct WorkingFiles;

namespace pipeline {
void reply(const RequestId &id, const std::function<void(JsonWriter &)> &fn);
void replyError(const RequestId &id,
                const std::function<void(JsonWriter &)> &fn);
} // namespace pipeline

struct CodeActionParam {
  TextDocumentIdentifier textDocument;
  lsRange range;
  struct Context {
    std::vector<Diagnostic> diagnostics;
  } context;
};
struct EmptyParam {};
struct DidOpenTextDocumentParam {
  TextDocumentItem textDocument;
};
struct RenameParam {
  TextDocumentIdentifier textDocument;
  Position position;
  std::string newName;
};
struct TextDocumentParam {
  TextDocumentIdentifier textDocument;
};
struct TextDocumentPositionParam {
  TextDocumentIdentifier textDocument;
  Position position;
};
struct TextDocumentEdit {
  VersionedTextDocumentIdentifier textDocument;
  std::vector<TextEdit> edits;
};
REFLECT_STRUCT(TextDocumentEdit, textDocument, edits);
struct WorkspaceEdit {
  std::vector<TextDocumentEdit> documentChanges;
};
REFLECT_STRUCT(WorkspaceEdit, documentChanges);

struct CallHierarchyItem {
  std::string name;
  SymbolKind kind;
  std::string detail;
  DocumentUri uri;
  lsRange range;
  lsRange selectionRange;
  std::string data;
};
REFLECT_STRUCT(CallHierarchyItem, name, kind, detail, uri, range,
               selectionRange, data);

struct CallsParam {
  CallHierarchyItem item;
};

// completion
enum class CompletionTriggerKind {
  Invoked = 1,
  TriggerCharacter = 2,
  TriggerForIncompleteCompletions = 3,
};
struct CompletionContext {
  CompletionTriggerKind triggerKind = CompletionTriggerKind::Invoked;
  std::optional<std::string> triggerCharacter;
};
struct CompletionParam : TextDocumentPositionParam {
  CompletionContext context;
};
enum class CompletionItemKind {
  Text = 1,
  Method = 2,
  Function = 3,
  Constructor = 4,
  Field = 5,
  Variable = 6,
  Class = 7,
  Interface = 8,
  Module = 9,
  Property = 10,
  Unit = 11,
  Value = 12,
  Enum = 13,
  Keyword = 14,
  Snippet = 15,
  Color = 16,
  File = 17,
  Reference = 18,
  Folder = 19,
  EnumMember = 20,
  Constant = 21,
  Struct = 22,
  Event = 23,
  Operator = 24,
  TypeParameter = 25,
};
enum class InsertTextFormat { PlainText = 1, Snippet = 2 };
struct CompletionItem {
  std::string label;
  CompletionItemKind kind = CompletionItemKind::Text;
  std::string detail;
  std::string documentation;
  std::string sortText;
  std::string filterText;
  InsertTextFormat insertTextFormat = InsertTextFormat::PlainText;
  TextEdit textEdit;
  std::vector<TextEdit> additionalTextEdits;

  std::vector<std::string> parameters_;
  int score_;
  unsigned priority_;
  int quote_kind_ = 0;
};

// formatting
struct FormattingOptions {
  int tabSize;
  bool insertSpaces;
};
struct DocumentFormattingParam {
  TextDocumentIdentifier textDocument;
  FormattingOptions options;
};
struct DocumentOnTypeFormattingParam {
  TextDocumentIdentifier textDocument;
  Position position;
  std::string ch;
  FormattingOptions options;
};
struct DocumentRangeFormattingParam {
  TextDocumentIdentifier textDocument;
  lsRange range;
  FormattingOptions options;
};

// workspace
enum class FileChangeType {
  Created = 1,
  Changed = 2,
  Deleted = 3,
};
struct DidChangeWatchedFilesParam {
  struct Event {
    DocumentUri uri;
    FileChangeType type;
  };
  std::vector<Event> changes;
