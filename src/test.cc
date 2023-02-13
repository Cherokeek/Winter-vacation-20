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
      rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
  writer.SetIndent(' ', 2);

  buffer.Clear();
  document.Accept(writer);
  return buffer.GetString();
}

struct TextReplacer {
  struct Replacement {
    std::string from;
    std::string to;
  };

  std::vector<Replacement> replacements;

  std::string apply(const std::string &content) {
    std::string result = content;

    for (const Replacement &replacement : replacements) {
      while (true) {
        size_t idx = result.find(replacement.from);
        if (idx == std::string::npos)
          break;

        result.replace(result.begin() + idx,
                       result.begin() + idx + replacement.from.size(),
                       replacement.to);
      }
    }

    return result;
  }
};

void trimInPlace(std::string &s) {
  auto f = [](char c) { return !isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), f));
  s.erase(std::find_if(s.rbegin(), s.rend(), f).base(), s.end());
}

std::vector<std::string> splitString(const std::string &str,
                                     const std::string &delimiter) {
  // http://stackoverflow.com/a/13172514
  std::vector<std::string> strings;

  std::string::size_type pos = 0;
  std::string::size_type prev = 0;
  while ((pos = str.find(delimiter, prev)) != std::string::npos) {
    strings.push_back(str.substr(prev, pos - prev));
    prev = pos + 1;
  }

  // To get the last substring (or only, if delimiter is not found)
  strings.push_back(str.substr(prev));

  return strings;
}

void parseTestExpectation(
    const std::string &filename,
    const std::vector<std::string> &lines_with_endings, TextReplacer *replacer,
    std::vector<std::string> *flags,
    std::unordered_map<std::string, std::string> *output_sections) {
  // Scan for EXTRA_FLAGS:
  {
    bool in_output = false;
    for (StringRef line : lines_with_endings) {
      line = line.trim();

      if (line.startswith("EXTRA_FLAGS:")) {
        assert(!in_output && "multiple EXTRA_FLAGS sections");
        in_output = true;
        continue;
      }

      if (in_output && line.empty())
        break;

      if (in_output)
        flags->push_back(line.str());
    }
  }

  // Scan for OUTPUT:
  {
    std::string active_output_filename;
    std::string active_output_contents;

    bool in_output = false;
    for (StringRef line_with_ending : lines_with_endings) {
      if (line_with_ending.startswith("*/"))
        break;

      if (line_with_ending.startswith("OUTPUT:")) {
        // Terminate the previous output section if we found a new one.
        if (in_output) {
          (*output_sections)[active_output_filename] = active_output_contents;
        }

        // Try to tokenize OUTPUT: based one whitespace. If there is more than
        // one token assume it is a filename.
        SmallVector<StringRef, 2> tokens;
        line_with_ending.split(tokens, ' ');
        if (tokens.size() > 1) {
          active_output_filename = tokens[1].str();
        } else {
          active_output_filename = filename;
        }
       