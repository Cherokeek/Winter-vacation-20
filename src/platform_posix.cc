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
#include <pthr