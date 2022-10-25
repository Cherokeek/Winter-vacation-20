// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>

#include <functional>
#include <string>

void getFilesInFolder(std::string folder, bool recursive,
                      bo