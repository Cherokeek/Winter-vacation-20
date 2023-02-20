
// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "utils.hh"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <tuple>
#include <utility>

// std::lock accepts two or more arguments. We define an overload for one
// argument.
namespace std {
template <typename Lockable> void lock(Lockable &l) { l.lock(); }
} // namespace std

namespace ccls {
struct BaseThreadQueue {
  virtual bool isEmpty() = 0;
  virtual ~BaseThreadQueue() = default;
};

template <typename... Queue> struct MultiQueueLock {
  MultiQueueLock(Queue... lockable) : tuple_{lockable...} { lock(); }
  ~MultiQueueLock() { unlock(); }
  void lock() { lock_impl(typename std::index_sequence_for<Queue...>{}); }
  void unlock() { unlock_impl(typename std::index_sequence_for<Queue...>{}); }

private:
  template <size_t... Is> void lock_impl(std::index_sequence<Is...>) {
    std::lock(std::get<Is>(tuple_)->mutex_...);
  }

  template <size_t... Is> void unlock_impl(std::index_sequence<Is...>) {
    (std::get<Is>(tuple_)->mutex_.unlock(), ...);
  }

  std::tuple<Queue...> tuple_;
};

struct MultiQueueWaiter {
  std::condition_variable_any cv;

  static bool hasState(std::initializer_list<BaseThreadQueue *> queues) {
    for (BaseThreadQueue *queue : queues) {
      if (!queue->isEmpty())
        return true;
    }
    return false;
  }

  template <typename... BaseThreadQueue>
  bool wait(std::atomic<bool> &quit, BaseThreadQueue... queues) {
    MultiQueueLock<BaseThreadQueue...> l(queues...);
    while (!quit.load(std::memory_order_relaxed)) {
      if (hasState({queues...}))