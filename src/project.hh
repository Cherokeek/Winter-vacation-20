// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "config.hh"
#include "lsp.hh"

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ccls {
struct WorkingFiles;

std::pair<LanguageId, bool> lookupExtension(std::string_view filename);

struct Project {
  struct Entry {
    std::string root;
    std::string directory;
    std::string filename;
    std::vector<const char *> args;
    // If true, this entry is inferred and was not read from disk.
    bool is_inferred = false;
    // 0 unless coming from a compile_commands.json entry.
    int compdb_size = 0;
    int id = -1;
  };

  struct Folder {
    std::string name;
    std::unordered_map<std::string, int> search_dir2kind;
    std::vector<Entry> entries;
    std::unordered_map<std::string, int> path2entry_index;
    std::unordered_map<std::string, std::vector<const char *>> dot_ccls;
  };

  std::mutex mtx;
  std::unordered_map<std::string, 