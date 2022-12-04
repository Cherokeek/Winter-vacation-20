// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "message_handler.hh"
#include "pipeline.hh"
#include "query.hh"

#include <llvm/Support/FormatVariadic.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <unordered_set>

namespace ccls {
namespace {
struct CodeAction {
  std::string title;
  const char *kind = "quickfix";
  WorkspaceEdit edit;
};
REFLECT_STRUCT(CodeAction, title, kind, edit);
} // namespace
void MessageHandler::textDocument_codeAction(CodeActionParam &param,
                                             ReplyOnce &reply) {
  WorkingFile *wf = findOrFail(param.textDocument.uri.getPath(), reply).second;
  if (!wf)
    return;
  std::vector<CodeAction> result;
  std::vector<Diagnostic> diagnostics;
  wfiles->withLock([&]() { diagnostics = wf->diagnostics; });
  for (Diagnostic &diag : diagnostics)
    if (diag.fixits_.size() &&
        (param.range.intersects(diag.range) ||
         llvm::any_of(diag.fixits_, [&](const TextEdit &edit) {
           return param.range.intersects(edit.range);
         }))) {
      CodeAction &cmd = result.emplace_back();
      cmd.title = "FixIt: " + diag.message;
      auto &edit =