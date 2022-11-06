// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "hierarchy.hh"
#include "message_handler.hh"
#include "pipeline.hh"
#include "query.hh"

#include <map>
#include <unordered_set>

namespace ccls {

namespace {

enum class CallType : uint8_t {
  Direct = 0,
  Base = 1,
  Derived = 2,
  All = 1 | 2
};
REFLECT_UNDERLYING(CallType);

bool operator&(CallType lhs, Ca