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
  // For unexpanded nodes, this is an upper bound because some entities may be
  // undefined. If it is 0, there are no members.
  int numChildren;
  // Empty if the |levels| limit is reached.
  std::vector<Out_cclsInheritance> children;
};
REFLECT_STRUCT(Out_cclsInheritance, id, kind, name, location, numChildren,
               children);

bool expand(MessageHandler *m, Out_cclsInheritance *entry, bool derived,
            bool qualified, int levels);

template <typename Q>
bool expandHelper(MessageHandler *m, Out_cclsInheritance *entry, bool derived,
                  bool qualified, int levels, Q &entity) {
  const auto *def = entity.anyDef();
  if (def) {
    entry->name = def->name(qualified);
    if (def->spell) {
      if (auto loc = getLsLocation(m->db, m->wfiles, *def->spell))
        entry->location = *loc;
    } else if (entity.declarations.size()) {
      if (auto loc = getLsLocation(m->db, m->wfiles, entity.declarations[0]))
        entry->location = *loc;
    }
  } else if (!derived) {
    entry->numChildren = 0;
    return false;
  }
  std::unordered_set<Usr> seen;
  if (derived) {
    if (levels > 0) {
      for (auto usr : entity.derived) {
        if (!seen.insert(usr).second)
          continue;
        Out_cclsInheritance entry1;
        entry1.id = std::to_string(usr);
        entry1.usr = usr;
        entry1.kind = entry->kind;
        if (expand(m, &entry1, derived, qualified, levels - 1))
          entry->children.push_back(std::move(entry1));
      }
      entry->numChildren = int(entry->children.size());
    } else
      entry->numChildren = int(entity.derived.size());
  } else {
    if (levels > 0) {
      for (auto usr : def->bases) {
        if (!seen.insert(usr).second)
          continue;
        Out_cclsInheritance entry1;
        entry1.id = std::to_string(usr);
  