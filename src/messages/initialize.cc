// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "filesystem.hh"
#include "include_complete.hh"
#include "log.hh"
#include "message_handler.hh"
#include "pipeline.hh"
#include "platform.hh"
#include "project.hh"
#include "sema_manager.hh"
#include "working_files.hh"

#include <llvm/ADT/Twine.h>
#include <llvm/Config/llvm-config.h>
#include <llvm/Support/Threading.h>

#include <rapidjson/