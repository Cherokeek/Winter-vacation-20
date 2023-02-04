// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "utils.hh"

#include <stdint.h>
#include <string>

namespace ccls {
struct Pos {
  uint16_t line = 0;
  int16_t column = -1;

  static Pos fromString(const std::string &encoded);

  bool valid() const { return column >= 0; }
  std::string toString();

  // Compare two Positions and check if they are equal. Ignores the value of
  // |interesting|.
  bool operator==(const Pos &o) const {
    return line == o.line && column == o.column;
  }
  bool operator<(const Pos &o) const {
    if (line != o.line)
      return line < o.line;
    return column < o.column;
  }
  bool operator<=(const Pos &o) const { return !(o < *this); }
};

struct Range {