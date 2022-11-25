// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "filesystem.hh"
#include "include_complete.hh"
#include "log.hh"
#include "message_handler.hh"
#include "pipeline.hh"
#include "platform.hh"
#include "project.hh"
#include "sema_manager.hh"
#include "working_files.hh"

#include <llvm/ADT/Twine.h>
#include <llvm/Config/llvm-config.h>
#include <llvm/Support/Threading.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <stdexcept>
#include <stdlib.h>
#include <thread>

namespace ccls {
using namespace llvm;

extern std::vector<std::string> g_init_options;

namespace {
enum class TextDocumentSyncKind { None = 0, Full = 1, Incremental = 2 };
REFLECT_UNDERLYING(TextDocumentSyncKind)

bool didChangeWatchedFiles;

struct ServerCap {
  struct SaveOptions {
    bool includeText = false;
  };
  struct TextDocumentSyncOptions {
    bool openClose = true;
    TextDocumentSyncKind change = TextDocumentSyncKind::Incremental;
    bool willSave = false;
    bool willSaveWaitUntil = false;
    SaveOptions save;
  } textDocumentSync;

  // The server provides hover support.
  bool hoverProvider = true;
  struct CompletionOptions {
    bool resolveProvider = false;

    // The characters that trigger completion automatically.
    // vscode doesn't support trigger character sequences, so we use ':'
    // for
    // '::' and '>' for '->'. See
    // https://github.com/Microsoft/language-server-protocol/issues/138.
    std::vector<const char *> triggerCharacters = {".", ":",  ">", "#",
                                                   "<", "\"", "/"};
  } completionProvider;
  struct SignatureHelpOptions {
    std::vector<const char *> triggerCharacters = {"(", ","};
  } signatureHelpProvider;
  bool declarationProvider = true;
  bool definitionProvider = true;
  bool typeDefinitionProvider = true;
  bool implementationProvider = true;
  bool referencesProvider = true;
  bool documentHighlightProvider = true;
  bool documentSymbolProvider = true;
  bool workspaceSymbolProvider = true;
  struct CodeActionOptions {
    std::vector<const char *> codeActionKinds = {"quickfix"};
  } codeActionProvider;
  struct CodeLensOptions {
    bool resolveProvider = false;
  } codeLensProvider;
  bool documentFormattingProvider = true;
  bool documentRangeFormattingProvider = true;
  Config::ServerCap::DocumentOnTypeFormattingOptions
      documentOnTypeFormattingProvider;
  bool renameProvider = true;
  struct DocumentLinkOptions {
    bool resolveProvider = true;
  } documentLinkProvider;
  bool foldingRangeProvider = true;
  // The server provides execute command support.
  struct ExecuteCommandOptions {
    std::vector<const char *> commands = {ccls_xref};
  } executeCommandProvider;
  bool callHierarchyProvider = true;
  Config::ServerCap::Workspace workspace;
};
REFLECT_STRUCT(ServerCap::CodeActionOptions, codeActionKinds);
REFLECT_STRUCT(ServerCap::CodeLensOptions, resolveProvider);
REFLECT_STRUCT(ServerCap::CompletionOptions, resolveProvider,
               triggerCharacters);
REFLECT_STRUCT(ServerCap::DocumentLinkOptions, resolveProvider);
REFLECT_STRUCT(ServerCap::ExecuteCommandOptions, commands);
REFLECT_STRUCT(ServerCap::SaveOptions, includeText);
REFLECT_STRUCT(ServerCap::SignatureHelpOptions, triggerCharacters);
REFLECT_STRUCT(ServerCap::TextDocumentSyncOptions, openClose, change, willSave,
               willSaveWaitUntil, save);
REFLECT_STRUCT(ServerCap, textDocumentSync, hoverProvider, completionProvider,
               signatureHelpProvider, declarationProvider, definitionProvider,
               implementationProvider, typeDefinitionProvider,
               referencesProvider, documentHighlightProvider,
               documentSymbolProvider, workspaceSymbolProvider,
               codeActionProvider, codeLensProvider, documentFormattingProvider,
               documentRangeFormattingProvider,
               documentOnTypeFormattingProvider, renameProvider,
               documentLinkProvider, foldingRangeProvider,
               executeCommandProvider, callHierarchyProvid