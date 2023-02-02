// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#if defined(__unix__) || defined(__APPLE__) || defined(__HAIKU__)
#include "platform.hh"

#include "utils.hh"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h> // required for stat.h
#include <sys/wait.h>
#include <unistd.h>
#ifdef __GLIBC__
#include <malloc.h>
#endif

#include <llvm/ADT/SmallString.h>
#include <llvm/Support/Path.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>

namespace ccls {
namespace pipeline {
void threadEnter();
}

std::string normalizePath(llvm::StringRef path) {
  llvm::SmallString<256> p(path);
  llvm::sys::path::remove_dots(p, true);
  return {p.data(), p.size()};
}

void fre