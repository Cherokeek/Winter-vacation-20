// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "test.hh"

#include "filesystem.hh"
#include "indexer.hh"
#include "pipeline.hh"
#include "platform.hh"
#include "sema_manager.hh"
#include "serializer.hh"
#include "utils.hh"

#include <llvm/ADT/StringRef.h>
#include <llvm/Config/llvm-config.h>

#include <rapidjson/document.h>
#include <rapidjson/prett