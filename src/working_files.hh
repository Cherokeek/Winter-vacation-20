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
  // Note: This assumes 0-based lines (1-based lines are normally assumed).
  std::vector<std::string> buffer_lines;
  // Mappings between index line number and buffer line number.
  // Empty indicates either buffer or index has been changed and re-computation
  // is required.
  // For index_to_buffer[i] == j, if j >= 0, we are confident that index line
  // i maps to buffer line j; if j == -1, FindMatchingLine will use the nearest
  // confident lines to resolve its line number.
  std::vector<int> index_to_buffer;
  std::vector<int> buffer_to_index;
  // A set of diagnostics that have been reported for this file.
  std::vector<Diagnostic> diagnostics;

  WorkingFile(const std::string &filename, const std::string &buffer_content);

  // This should be called when the indexed content has changed.
  void setIndexContent(const std::string &index_content);
  // This should be called whenever |buffer_content| has changed.
  void onBufferContentUpdated();

  // Finds the buffer line number which maps to index line number |line|.
  // Also resolves |column| if not NULL.
  // When resolving a range, use is_end = false for begin() and is_end =
  // true for end() to get a better alignment of |column|.
  std::optional<int> getBufferPosFromIndexPos(int line, i