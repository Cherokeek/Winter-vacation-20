// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "pipeline.hh"

#include "config.hh"
#include "include_complete.hh"
#include "log.hh"
#include "lsp.hh"
#include "message_handler.hh"
#include "pipeline.hh"
#include "platform.hh"
#include "project.hh"
#include "query.hh"
#include "sema_manager.hh"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/Threading.h>

#include <chrono>
#include <inttypes.h>
#include <mutex>
#include <shared_mutex>
#include <thread>
#ifndef _WIN32
#include <unistd.h>
#endif
using namespace llvm;
namespace chrono = std::chrono;

namespace ccls {
namespace {
struct PublishDiagnosticParam {
  DocumentUri uri;
  std::vector<Diagnostic> diagnostics;
};
REFLECT_STRUCT(PublishDiagnosticParam, uri, diagnostics);

constexpr char index_progress_token[] = "index";
struct WorkDoneProgressCreateParam {
  const char *token = index_progress_token;
};
REFLECT_STRUCT(WorkDoneProgressCreateParam, token);
} // namespace

void VFS::clear() {
  std::lock_guard lock(mutex);
  state.clear();
}

int VFS::loaded(const std::string &path) {
  std::lock_guard lock(mutex);
  return state[path].loaded;
}

bool VFS::stamp(const std::string &path, int64_t ts, int step) {
  std::lock_guard<std::mutex> lock(mutex);
  State &st = state[path];
  if (st.timestamp < ts || (st.timestamp == ts && st.step < step)) {
    st.timestamp = ts;
    st.step = step;
    return true;
  } else
    return false;
}

struct MessageHandler;
void standaloneInitialize(MessageHandler &, const std::string &root);

namespace pipeline {

std::atomic<bool> g_quit;
std::atomic<int64_t> loaded_ts{0}, request_id{0};
IndexStats stats;
int64_t tick = 0;

namespace {

struct IndexRequest {
  std::string path;
  std::vector<const char *> args;
  IndexMode mode;
  bool must_exist = false;
  RequestId id;
  int64_t ts = tick++;
};

std::mutex thread_mtx;
std::condition_variable no_active_threads;
int active_threads;

MultiQueueWaiter *main_waiter;
MultiQueueWaiter *indexer_waiter;
MultiQueueWaiter *stdout_waiter;
ThreadedQueue<InMessage> *on_request;
ThreadedQueue<IndexRequest> *index_request;
ThreadedQueue<IndexUpdate> *on_indexed;
ThreadedQueue<std::string> *for_stdout;

struct InMemoryIndexFile {
  std::string content;
  IndexFile index;
};
std::shared_mutex g_index_mutex;
std::unordered_map<std::string, InMemoryIndexFile> g_index;

bool cacheInvalid(VFS *vfs, IndexFile *prev, const std::string &path,
                  const std::vector<const char *> &args,
                  const std::optional<std::string> &from) {
  {
    std::lock_guard<std::mutex> lock(vfs->mutex);
    if (prev->mtime < vfs->state[path].timestamp) {
      LOG_V(1) << "timestamp changed for " << path
               << (from ? " (via " + *from + ")" : std::string());
      return true;
    }
  }

  // For inferred files, allow -o a a.cc -> -o b b.cc
  StringRef stem = sys::path::stem(path);
  int changed = -1, size = std::min(prev->args.size(), args.size());
  for (int i = 0; i < size; i++)
    if (strcmp(prev->args[i], args[i]) && sys::path::stem(args[i]) != stem) {
      changed = i;
      break;
    }
  if (changed < 0 && prev->args.size() != args.size())
    changed = size;
  if (changed >= 0)
    LOG_V(1) << "args changed for " << path
             << (from ? " (via " + *from + ")" : std::string()) << "; old: "
             << (changed < prev->args.size() ? prev->args[changed] : "")
             << "; new: " << (changed < size ? args[changed] : "");
  return changed >= 0;
};

std::string appendSerializationFormat(const std::string &base) {
  switch (g_config->cache.format) {
  case SerializeFormat::Binary:
    return base + ".blob";
  case SerializeFormat::Json:
    return base + ".json";
  }
}

std::string getCachePath(std::string src) {
  if (g_config->cache.hierarchicalPath) {
    std::string ret = src[0] == '/' ? src.substr(1) : src;
#ifdef _WIN32
    std::replace(ret.begin(), ret.end(), ':', '@');
#endif
    return g_config->cache.directory + ret;
  }
  for (auto &[root, _] : g_config->workspaceFolders)
    if (StringRef(src).startswith(root)) {
      auto len = root.size();
      return g_config->cache.directory +
             escapeFileName(root.substr(0, len - 1)) + '/' +
             escapeFileName(src.substr(len));
    }
  return g_config->cache.directory + '@' +
         escapeFileName(g_config->fallbackFolder.substr(
             0, g_config->fallbackFolder.size() - 1)) +
         '/' + escapeFileName(src);
}

std::unique_ptr<IndexFile> rawCacheLoad(const std::string &path) {
  if (g_config->cache.retainInMemory) {
    std::shared_lock lock(g_index_mutex);
    auto it = g_index.find(path);
    if (it != g_index.end())
      return std::make_unique<IndexFile>(it->second.index);
    if (g_config->cache.directory.empty())
      return nullptr;
  }

  std::string cache_path = getCachePath(path);
  std::optional<std::string> file_content = readContent(cache_path);
  std::optional<std::string> serialized_indexed_content =
      readContent(appendSerializationFormat(cache_path));
  if (!file_content || !serialized_indexed_content)
    return nullptr;

  return ccls::deserialize(g_config->cache.format, path,
                           *serialized_indexed_content, *file_content,
                           IndexFile::kMajorVersion);
}

std::mutex &getFileMutex(const std::string &path) {
  const int n_MUTEXES = 256;
  static std::mutex mutexes[n_MUTEXES];
  return mutexes[std::hash<std::string>()(path) % n_MUTEXES];
}

bool indexer_Parse(SemaManager *completion, WorkingFiles *wfiles,
                   Project *project, VFS *vfs, const GroupMatch &matcher) {
  std::optional<IndexRequest> opt_request = index_request->tryPopFront();
  if (!opt_request)