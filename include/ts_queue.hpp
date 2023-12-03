#ifndef EXSTD_TSQUEUE_HPP
#define EXSTD_TSQUEUE_HPP

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

template <typename T> struct ts_queue {
  std::queue<T> internal;
  std::mutex access_mutex;
  std::condition_variable cond;

  void push(const T &value);
  std::optional<T> pop();
  bool empty();

  ~ts_queue() { cond.notify_all(); }
};

template <typename T> void ts_queue<T>::push(const T &value) {
  {
    std::unique_lock<std::mutex> lock(access_mutex);
    internal.push(value);
  }
  cond.notify_one();
}

template <typename T> std::optional<T> ts_queue<T>::pop() {
  std::unique_lock<std::mutex> lock(access_mutex);
  cond.wait(lock, [this] { return !internal.empty(); });
  if (internal.empty()) {
    return std::nullopt;
  }

  T t = internal.front();
  internal.pop();
  return t;
}

template <typename T> bool ts_queue<T>::empty() {
  std::lock_guard<std::mutex> lock(access_mutex);
  return internal.empty();
}

#endif
