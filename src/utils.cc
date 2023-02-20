// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "utils.hh"

#include "log.hh"
#include "message_handler.hh"
#include "pipeline.hh"
#include "platform.hh"

#include <siphash.h>

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>

#include <algorithm>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <functional>
#include <regex>
#include <string.h>
#include <unordered_map>

using namespace llvm;

namespace ccls {
struct Matcher::Impl {
  std::regex regex;
};

Matcher::Matcher(const std::string &pattern)
    : impl(std::make_unique<Impl>()), pattern(pattern) {
  impl->regex = std::regex(pattern, std::regex_constants::ECMAScript |
                                        std::regex_constants::icase |
                                        std::regex_constants::optimize);
}

Matcher::~Matcher() {}

bool Matcher::matches(const std::string &text) const {
  return std::regex_search(text, impl->regex, std::regex_constants::match_any);
}

GroupMatch::GroupMatch(const std::vector<std: