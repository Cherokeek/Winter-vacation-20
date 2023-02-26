// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "lsp.hh"
#include "utils.hh"

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace ccls {
struct WorkingFile {
  int64_t timestamp = 0;
  int version = 0;
  std::string filename;

  std::string buffer_content;
  // Note: This assumes 0-based lines (1-based lines are normally assumed).
  std::vector<std::string> index_lines;
  // Note: This assumes 0-based lines (1-based l