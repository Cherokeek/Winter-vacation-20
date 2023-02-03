// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "position.hh"

#include "serializer.hh"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <algorithm>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

namespace ccls {
Pos Pos::fromString(const std::string &encoded) {
  char *p = const_cast<char *>(encoded.c_str());
  uint16_t line = uint16_t(strtoul(p, &p, 10) - 1);
  assert(*p == ':');
  p++;
  int16_t column = int16_t(strtol(p, &p, 10)) - 1;
  return {line, column};
}

std::string Pos::toString() {
  char buf[99];
  snprintf(buf, sizeof buf, "%d:%d", line + 1, column + 1);
  return buf;
}

Range Range::fromString(const std::string &encoded) {
  Pos start, end;
  char *p = const_cast<char *>(encoded.c_str());
  start.line = uint16_t(strtoul(p, &p, 10) - 1);
  assert(*p == ':');
  p++;
  start.column = int16_t(strtol(p, &p, 10)) - 1;
  assert(*p == '-');
  p++;

  end.line = uint16_t(strtoul(p, &p, 10) - 1);
  assert(*p == ':');
  p++;
  end.column = int16_t(strtol(p, nullptr, 10)) - 1;
  return {start, end};
}

bool Range::contains(int line, int column) const {
  if (line > UINT16_MAX)
    return false;
  Pos p{(uint16_t)line, (int16_t)std::min<int>(column, INT16_MAX)};
  return !(p < start) && p < end;
}

std::string Range::toString() {
  char buf[99];
  snprintf(buf, sizeof buf, "%d:%d-%d:%d", start.line + 1, start.column + 1,
           end.line + 1, end.column + 1);
  return buf;
