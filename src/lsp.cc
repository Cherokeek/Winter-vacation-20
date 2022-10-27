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
    v.value = std::to_string(vis.m->GetInt());
  } else if (vis.m->IsString()) {
    v.type = RequestId::kString;
    v.value = vis.m->GetString();
  } else {
    v.type = RequestId::kNone;
    v.value.clear();
  }
}

void reflect(JsonWriter &visitor, RequestId &value) {
  switch (value.type) {
  case RequestId::kNone:
    visitor.null_();
    break;
  case RequestId::kInt:
    visitor.int64(atoll(value.value.c_str()));
    break;
  case RequestId::kString:
    visitor.string(value.value.c_str(), value.value.size());
    break;
  }
}

DocumentUri DocumentUri::fromPath(const std::string &path) {
  DocumentU