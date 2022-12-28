// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "include_complete.hh"
#include "message_handler.hh"
#include "pipeline.hh"
#include "project.hh"
#include "sema_manager.hh"
#include "working_files.hh"

namespace ccls {
void MessageHandler::textDocument_didChange(TextDocumentDidChangeParam &param) {
  std::string path = param.textDocument.uri.getPath();
  wfiles->onChange(param);
  if (g_config->index.onChange)
    pipeline::index(path, {}, IndexMode::OnChange, true);
  manager->onView(path);
  if (g_config->diagnostics.onChange >= 0)
    manager->scheduleDiag(path, g_config->diagnostics.onChange);
}

void MessageHandler::textDocument_didClose(TextDocumentParam &param) {
  std::string path = param.textDocument.uri.getPath();
  wfiles->onClose(path);
  manager->onClose(path);
  pipeline::removeCache(path);
}