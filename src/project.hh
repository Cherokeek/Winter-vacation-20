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
    std::string directo