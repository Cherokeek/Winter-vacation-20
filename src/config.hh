
// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "serializer.hh"

#include <string>

namespace ccls {
/*
The language client plugin needs to send initialization options in the
`initialize` request to the ccls language server.

If necessary, the command line option --init can be used to override
initialization options specified by the client. For example, in shell syntax:

  '--init={"index": {"comments": 2, "whitelist": ["."]}}'
*/
struct Config {
  // **Not available for configuration**
  std::string fallbackFolder;
  std::vector<std::pair<std::string, std::string>> workspaceFolders;
  // If specified, this option overrides compile_commands.json and this
  // external command will be executed with an option |projectRoot|.
  // The initialization options will be provided as stdin.
  // The stdout of the command should be the JSON compilation database.
  std::string compilationDatabaseCommand;
  // Directory containing compile_commands.json.
  std::string compilationDatabaseDirectory;

  struct Cache {
    // Cache directory for indexed files, either absolute or relative to the
    // project root.
    //
    // If empty, retainInMemory will be set to 1 and cache will be stored in
    // memory.
    std::string directory = ".ccls-cache";

    // Cache serialization format.
    //
    // "json" generates $directory/.../xxx.json files which can be pretty
    // printed with jq.
    //
    // "binary" uses a compact binary serialization format.
    // It is not schema-aware and you need to re-index whenever an internal
    // struct member has changed.
    SerializeFormat format = SerializeFormat::Binary;

    // If false, store cache files as $directory/@a@b/c.cc.blob
    //
    // If true, $directory/a/b/c.cc.blob. If cache.directory is absolute, make
    // sure different projects use different cache.directory as they would have
    // conflicting cache files for system headers.
    bool hierarchicalPath = false;

    // After this number of loads, keep a copy of file index in memory (which
    // increases memory usage). During incremental updates, the index subtracted
    // will come from the in-memory copy, instead of the on-disk file.
    //
    // The initial load or a save action is counted as one load.
    // 0: never retain; 1: retain after initial load; 2: retain after 2 loads
    // (initial load+first save)
    int retainInMemory = 2;
  } cache;

  struct ServerCap {
    struct DocumentOnTypeFormattingOptions {
      std::string firstTriggerCharacter = "}";
      std::vector<const char *> moreTriggerCharacter;
    } documentOnTypeFormattingProvider;

    // Set to false if you don't want folding ranges.
    bool foldingRangeProvider = true;

    struct Workspace {
      struct WorkspaceFolders {
        // Set to false if you don't want workspace folders.
        bool supported = true;

        bool changeNotifications = true;
      } workspaceFolders;
    } workspace;
  } capabilities;

  struct Clang {
    // Arguments matching any of these glob patterns will be excluded, e.g.
    // ["-fopenmp", "-m*", "-Wall"].
    std::vector<std::string> excludeArgs;

    // Additional arguments to pass to clang.
    std::vector<std::string> extraArgs;

    // Translate absolute paths in compile_commands.json entries, .ccls options
    // and cache files. This allows to reuse cache files built otherwhere if the
    // source paths are different.
    //
    // This is a list of colon-separated strings, e.g. ["/container:/host"]
    //
    // An entry of "clang -I /container/include /container/a.cc" will be
    // translated to "clang -I /host/include /host/a.cc". This is simple string
    // replacement, so "clang /prefix/container/a.cc" will become "clang
    // /prefix/host/a.cc".
    std::vector<std::string> pathMappings;

    // Value to use for clang -resource-dir if not specified.
    //
    // This option defaults to clang -print-resource-dir and should not be
    // specified unless you are using an esoteric configuration.
    std::string resourceDir;
  } clang;

  struct ClientCapability {
    // TextDocumentClientCapabilities.publishDiagnostics.relatedInformation
    bool diagnosticsRelatedInformation = true;
    // TextDocumentClientCapabilities.documentSymbol.hierarchicalDocumentSymbolSupport