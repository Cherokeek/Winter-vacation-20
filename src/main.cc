// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "log.hh"
#include "pipeline.hh"
#include "platform.hh"
#include "serializer.hh"
#include "test.hh"
#include "working_files.hh"

#include <clang/Basic/Version.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/CrashRecoveryContext.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/Signals.h>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

using namespace ccls;
using namespace llvm;
using namespace llvm::cl;

namespace ccls {
std::vector<std::string> g_init_options;
}

namespace {
OptionCategory C("ccls options");

opt<bool> opt_help("h", desc("Alias for -help"), cat(C));
opt<int> opt_verbose("v", desc("verbosity, from -3 (fatal) to 2 (verbose)"),
                     init(0), cat(C));
opt<std::string> opt_test_index("test-index", ValueOptional, init("!"),
                                desc("run index tests"), cat(C));

opt<std::string> opt_index("index",
                           desc("standalone mode: index a project and exit"),
                           value_desc("root"), cat(C));
list<std::string> opt_init("init", desc("extra initialization options in JSON"),
                           cat(C));
opt<std::string> opt_log_file("log-file", desc("stderr or log file"),
                              value_desc("file"), init("stderr"), cat(C));
opt<bool> opt_log_file_append("log-file-append", desc("append to log file"),
                              cat(C