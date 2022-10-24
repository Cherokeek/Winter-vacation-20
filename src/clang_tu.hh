// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "position.hh"

#include <clang/Basic/FileManager.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>

#if LLVM_VERSION_MAJOR < 8
// D52783 Lift VFS from clang to llvm
namespace llvm {
namespace vfs = clang::vfs;
}
#endif

#if LLVM_VERSION_MAJOR < 14 // llvmorg-14-init-3863-g601102d282d5
#define isAsciiIdentifierContinue isIdentifierBody