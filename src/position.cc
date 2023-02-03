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
  assert(*p == ':