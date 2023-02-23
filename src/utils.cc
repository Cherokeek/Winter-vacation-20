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

GroupMatch::GroupMatch(const std::vector<std::string> &whitelist,
                       const std::vector<std::string> &blacklist) {
  auto err = [](const std::string &pattern, const char *what) {
    ShowMessageParam params;
    params.type = MessageType::Error;
    params.message =
        "failed to parse EMCAScript regex " + pattern + " : " + what;
    pipeline::notify(window_showMessage, params);
  };
  for (const std::string &pattern : whitelist) {
    try {
      this->whitelist.emplace_back(pattern);
    } catch (const std::exception &e) {
      err(pattern, e.what());
    }
  }
  for (const std::string &pattern : blacklist) {
    try {
      this->blacklist.emplace_back(pattern);
    } catch (const std::exception &e) {
      err(pattern, e.what());
    }
  }
}

bool GroupMatch::matches(const std::string &text,
                         std::string *blacklist_pattern) const {
  for (const Matcher &m : whitelist)
    if (m.matches(text))
      return true;
  for (const Matcher &m : blacklist)
    if (m.matches(text)) {
      if (blacklist_pattern)
        *blacklist_pattern = m.pattern;
      return false;
    }
  return true;
}

uint64_t hashUsr(llvm::StringRef s) {
  union {
    uint64_t ret;
    uint8_t out[8];
  };
  // k is an arbitrary key. Don't change it.
  const uint8_t k[16] = {0xd0, 0xe5, 0x4d, 0x61, 0x74, 0x63, 0x68, 0x52,
                         0x61, 0x79, 0xea, 0x70, 0xca, 0x70, 0xf0, 0x0d};
  (void)siphash(reinterpret_cast<const uint8_t *>(s.data()), s.size(), k, out,
                8);
  return ret;
}

std::string lowerPathIfInsensitive(const std::string &path) {
#if defined(_WIN32)
  std::string ret = path;
  for (char &c : ret)
    c = tolower(c);
  return ret;
#else
  return path;
#endif
}

void ensureEndsInSlash(std::string &path) {
  if (path.empty() || path[path.size() - 1] != '/')
    path += '/';
}

std::string es