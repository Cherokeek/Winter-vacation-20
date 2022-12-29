// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "message_handler.hh"
#include "query.hh"

#include <clang/Basic/CharInfo.h>

#include <unordered_set>

using namespace clang;

namespace ccls {
namespace {
WorkspaceEdit buildWorkspaceEdit(DB *db, WorkingFiles *wfiles, SymbolRef sym,
                                 std::string_view old_text,
                                 const std::string &new_text) {
  std::unordered_map<int, std::pair<WorkingFile *, TextDocumentEdit>> path2edit;
  std::unordered_map<int, std::unordered_set<Range>> edited;

  eachOccurrence(db, sym, true, [&](Use use) {
    int file_id