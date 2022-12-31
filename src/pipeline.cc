// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "pipeline.hh"

#include "config.hh"
#include "include_complete.hh"
#include "log.hh"
#include "lsp.hh"
#include "message_handler.hh"
#include "pipeline.hh"
#include "platform.hh"
#include "project.hh"
#include "query.hh"
#include "sema_manager.hh"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/Threading.h>

#include <chrono>
#include <inttypes.h>
#include <mutex>
#include <shared_mutex>
#include <thread>
#ifndef _WIN32
#include <unistd.h>
#endif
using namespace llvm;
namespace chrono = std::chrono;

namespace ccls {
namespace {
struct PublishDiagnosticParam {
  DocumentUri uri;
  std::vector<Diagnostic> diagnostics;
};
REFLECT_STRUCT(PublishDiagnosticParam, uri, diagnostics);

constexpr char index_progress_token[] = "index";
struct WorkDoneProgressCreateParam {
  const char *token = index_