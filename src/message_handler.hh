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
struct Completio