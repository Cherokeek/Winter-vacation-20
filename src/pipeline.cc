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
    return false;
  auto &request = *opt_request;
  bool loud = request.mode != IndexMode::OnChange;

  // Dummy one to trigger refresh semantic highlight.
  if (request.path.empty()) {
    IndexUpdate dummy;
    dummy.refresh = true;
    on_indexed->pushBack(std::move(dummy), false);
    return false;
  }

  struct RAII {
    ~RAII() { stats.completed++; }
  } raii;
  if (!matcher.matches(request.path)) {
    LOG_IF_S(INFO, loud) << "skip " << request.path;
    return false;
  }

  Project::Entry entry =
      project->findEntry(request.path, true, request.must_exist);
  if (request.must_exist && entry.filename.empty())
    return true;
  if (request.args.size())
    entry.args = request.args;
  std::string path_to_index = entry.filename;
  std::unique_ptr<IndexFile> prev;

  bool deleted = request.mode == IndexMode::Delete,
       no_linkage = g_config->index.initialNoLinkage ||
                    request.mode != IndexMode::Background;
  int reparse = 0;
  if (deleted)
    reparse = 2;
  else if (!(g_config->index.onChange && wfiles->getFile(path_to_index))) {
    std::optional<int64_t> write_time = lastWriteTime(path_to_index);
    if (!write_time) {
      deleted = true;
    } else {
      if (vfs->stamp(path_to_index, *write_time, no_linkage ? 2 : 0))
        reparse = 1;
      if (request.path != path_to_index) {
        std::optional<int64_t> mtime1 = lastWriteTime(request.path);
        if (!mtime1)
          deleted = true;
        else if (vfs->stamp(request.path, *mtime1, no_linkage ? 2 : 0))
          reparse = 2;
      }
    }
  }

  if (g_config->index.onChange) {
    reparse = 2;
    std::lock_guard lock(vfs->mutex);
    vfs->state[path_to_index].step = 0;
    if (request.path != path_to_index)
      vfs->state[request.path].step = 0;
  }
  bool track = g_config->index.trackDependency > 1 ||
               (g_config->index.trackDependency == 1 && request.ts < loaded_ts);
  if (!reparse && !track)
    return true;

  if (reparse < 2)
    do {
      std::unique_lock lock(getFileMutex(path_to_index));
      prev = rawCacheLoad(path_to_index);
      if (!prev || prev->no_linkage < no_linkage ||
          cacheInvalid(vfs, prev.get(), path_to_index, entry.args,
                       std::nullopt))
        break;
      if (track)
        for (const auto &dep : prev->dependencies) {
          if (auto mtime1 = lastWriteTime(dep.first.val().str())) {
            if (dep.second < *mtime1) {
              reparse = 2;
              LOG_V(1) << "timestamp changed for " << path_to_index << " via "
                       << dep.first.val().str();
              break;
            }
          } else {
            reparse = 2;
            LOG_V(1) << "timestamp changed for " << path_to_index << " via "
                     << dep.first.val().str();
            break;
          }
        }
      if (reparse == 0)
        return true;
      if (reparse == 2)
        break;

      if (vfs->loaded(path_to_index))
        return true;
      LOG_S(INFO) << "load cache for " << path_to_index;
      auto dependencies = prev->dependencies;
      IndexUpdate update = IndexUpdate::createDelta(nullptr, prev.get());
      on_indexed->pushBack(std::move(update),
                           request.mode != IndexMode::Background);
      {
        std::lock_guard lock1(vfs->mutex);
        VFS::State &st = vfs->state[path_to_index];
        st.loaded++;
        if (prev->no_linkage)
          st.step = 2;
      }
      lock.unlock();

      for (const auto &dep : dependencies) {
        std::string path = dep.first.val().str();
        if (!vfs->stamp(path, dep.second, 1))
          continue;
        std::lock_guard lock1(getFileMutex(path));
        prev = rawCacheLoad(path);
        if (!prev)
          continue;
        {
          std::lock_guard lock2(vfs->mutex);
          VFS::State &st = vfs->state[path];
          if (st.loaded)
            continue;
          st.loaded++;
          st.timestamp = prev->mtime;
          if (prev->no_linkage)
            st.step = 3;
        }
        IndexUpdate update = IndexUpdate::createDelta(nullptr, prev.get());
        on_indexed->pushBack(std::move(update),
                             request.mode != IndexMode::Background);
        if (entry.id >= 0) {
          std::lock_guard lock2(project->mtx);
          project->root2folder[entry.root].path2entry_index[path] = entry.id;
        }
      }
      return true;
    } while (0);

  std::vector<std::unique_ptr<IndexFile>> indexes;
  int n_errs = 0;
  std::string first_error;
  if (deleted) {
    indexes.push_back(std::make_unique<IndexFile>(request.path, "", false));
    if (request.path != path_to_index)
      indexes.push_back(std::make_unique<IndexFile>(path_to_index, "", false));
  } else {
    std::vector<std::pair<std::string, std::string>> remapped;
    if (g_config->index.onChange) {
      std::string content = wfiles->getContent(path_to_index);
      if (content.size())
        remapped.emplace_back(path_to_index, content);
    }
    bool ok;
    auto result =
        idx::index(completion, wfiles, vfs, entry.directory, path_to_index,
                   entry.args, remapped, no_linkage, ok);
    indexes = std::move(result.indexes);
    n_errs = result.n_errs;
    first_error = std::move(result.first_error);

    if (!ok) {
      if (request.id.valid()) {
        ResponseError err;
        err.code = ErrorCode::InternalError;
        err.message = "failed to index " + path_to_index;
        pipeline::replyError(request.id, err);
      }
      return true;
    }
  }

  if (loud || n_errs) {
    std::string line;
    SmallString<64> tmp;
    SmallString<256> msg;
    (Twine(deleted ? "delete " : "parse ") + path_to_index).toVector(msg);
    if (n_errs)
      msg += " error:" + std::to_string(n_errs) + ' ' + first_error;
    if (LOG_V_ENABLED(1)) {
      msg += "\n ";
      for (const char *arg : entry.args)
        (msg += ' ') += arg;
    }
    LOG_S(INFO) << std::string_view(msg.data(), msg.size());
  }

  for (std::unique_ptr<IndexFile> &curr : indexes) {
    std::string path = curr->path;
    if (!matcher.matches(path)) {
      LOG_IF_S(INFO, loud) << "skip index for " << path;
      continue;
    }

    if (!deleted)
      LOG_IF_S(INFO, loud) << "store index for " << path
                           << " (delta: " << !!prev << ")";
    {
      std::lock_guard lock(getFileMutex(path));
      int loaded = vfs->loaded(path), retain = g_config->cache.retainInMemory;
      if (loaded)
        prev = rawC