// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "test.hh"

#include "filesystem.hh"
#include "indexer.hh"
#include "pipeline.hh"
#include "platform.hh"
#include "sema_manager.hh"
#include "serializer.hh"
#include "utils.hh"

#include <llvm/ADT/StringRef.h>
#include <llvm/Config/llvm-config.h>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <fstream>
#include <stdio.h>
#include <stdlib.h>

// The 'diff' utility is available and we can use dprintf(3).
#if _POSIX_C_SOURCE >= 200809L
#include <sys/wait.h>
#include <unistd.h>
#endif

using namespace llvm;

extern bool gTestOutputMode;

namespace ccls {
std::string toString(const rapidjson::Document &document) {
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  writer.SetFormatOptions(
      rapidjson::Prett