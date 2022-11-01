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
  str