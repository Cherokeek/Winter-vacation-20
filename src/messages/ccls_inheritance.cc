// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "hierarchy.hh"
#include "message_handler.hh"
#include "pipeline.hh"
#include "query.hh"

#include <unordered_set>

namespace ccls {
namespace {
struct Param : TextDocumentPositionParam {
  // If id+kind are specified, expand a node; otherwise textDocument+position
  // should be specified for building the root and |levels| of nodes below.
  Usr usr;
  std::string id;
  Kind kind = Kind::Invalid;

  // true: derived classes/functions; false: base classes/functions
  bool derived = false;
  bool qualified = true;
  int levels = 1;
  bool hierarchy = false;
};

REFLECT_STRUCT(Param, textDocument, position, id, kind, derived, qualified,
               levels, hierarchy);

struct Out_cclsInheritance {
  Usr usr;
  std::string id;
  Kind kind;
  std::string_view name;
  Location location;
  // For unexpanded nodes, this is an upper 