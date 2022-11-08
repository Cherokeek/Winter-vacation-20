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
  bool hierarchy = false;
};
REFLECT_STRUCT(Param, textDocument, position, id, callee, callType, qualified,
               levels, hierarchy);

struct Out_cclsCall {
  Usr usr;
  std::string id;
  std::string_view name;
  Location location;
  CallType callType = CallType::Direct;
  int numChildren;
  // Empty if the |levels| limit is reached.
  std::vector<Out_cclsCall> children;
  bool operator==(const Out_cclsCall &o) const {
    return location == o.location;
  }
  bool operator<(const Out_cclsCall &o) const { return location < o.location; }
};
REFLECT_STRUCT(Out_cclsCall, id, name, location, callType, numChildren,
               children);

struct Out_incomingCall {
  CallHierarchyItem from;
  std::vector<lsRange> fromRanges;
};
REFLECT_STRUCT(Out_incomingCall, from, fromRanges);

struct Out_outgoingCall {
  CallHierarchyItem to;
  std::vector<lsRange> fromRanges;
};
REFLECT_STRUCT(Out_outgoingCall, to, fromRanges);

bool expand(MessageHandler *m, Out_cclsCall *entry, bool callee,
            CallType call_type, bool qualified, int levels) {
  const QueryFunc &func = m->db->getFunc(entry->usr);
  const QueryFunc::Def *def = func.anyDef();
  entry->numChildren = 0;
  if (!def)
    return false;
  auto handle = [&](SymbolRef sym, int file_id, CallType call_type1) {
    entry->numChildren++;
    if (levels > 0) {
      Out_cclsCall entry1;
      entry1.id = std::to_string(sym.usr);
      entry1.usr = sym.usr;
      if (auto loc = getLsLocation(m->db, m->wfiles,
                                   Use{{sym.range, sym.role}, file_id}))
        entry1.location = *loc;
      entry1.callType = call_type1;
      if (expand(m, &entry1, callee, call_type, qualified, levels - 1))
        entry->children.push_back(std::move(entry1));
    }
  };
  auto handle_uses = [&](const QueryFunc &func, CallType call_type) {
    if (callee) {
      if (const auto *def = func.anyDef())
        for (SymbolRef sym : def->callees)
          if (sym.kind == Kind::Func)
            handle(sym, def->file_id, call_type);
    } else {
      for (Use use : func.uses) {
        const QueryFile &file1 = m->db->files[use.file_id];
        Maybe<ExtentRef> best;
        for (auto [sym, refcnt] : file1.symbol2refcnt)
          if (refcnt > 0 && sym.extent.valid() && sym.kind == Kind::Func &&
              sym.extent.start <= use.range.start &&
              use.range.end <= sym.extent.end &&
              (!best || best->extent.start < sym.extent.start))
            best = sy