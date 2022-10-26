// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "lsp.hh"

#include "log.hh"

#include <rapidjson/document.h>

#include <algorithm>
#include <stdio.h>

namespace ccls {
void reflect(JsonReader &vis, RequestId &v) {
  if (vis.m->IsInt64()) {
    v.type = RequestId::kInt;
    v.value = std::to_string(int(vis.m->GetInt64()));
  } else if (vis.m->IsInt()) {
    v.type = RequestId::kInt;
    v.value = std::to_st