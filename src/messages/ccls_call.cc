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

bool operator&(CallType lhs, CallType rhs) {
  return uint8_t(lhs) & uint8_t(rhs);
}

struct Param : TextDocumentPositionParam {
  // If id is specified, expand a node; otherwise textDocument+position should
  // be specified for building the root and |levels| of nodes below.
  Usr usr;
  std::string id;

  // true: callee tree (functions called by this function); false: caller tree
  // (where this function is called)
  bool callee = false;
  // Base: include base functions; All: include both base and derived
  // functions.
  CallType callType = CallType::All;
  bool qualified = true;
  int levels = 1;
  bool hierarchy = fals