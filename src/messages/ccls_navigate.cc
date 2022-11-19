// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "message_handler.hh"
#include "query.hh"

namespace ccls {
namespace {
struct Param {
  TextDocumentIdentifier textDocument;
  Position position;
  std::string direction;
};
REFLECT_STRUCT(Param, textDocument, position, direction);

Maybe<Range> findParent(QueryFile *file, Pos pos) {
  Maybe<Range> parent;
  for (auto [sym, refcnt] : file->symbol2refcnt)
    if (refcnt > 0 && sym.extent.valid() && sym.extent.start <= pos &&
        pos < sym.extent.end &&
        (!parent || (parent->start == sym.extent.start
                         ? parent->end < sym.extent.end
                         : parent->start < sym.extent.start)))
      parent = sym.extent;
  