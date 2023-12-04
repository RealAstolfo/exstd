#ifndef EXSTD_TSQUEUE_HPP
#define EXSTD_TSQUEUE_HPP

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

/**
 * @file ts_queue.hpp
 * @brief Definition of the ts_queue template struct.
 */

/**
 * @brief A template struct for a thread-safe queue.
 *
 * The ts_queue template struct provides a thread-safe implementation of a
 * queue, allowing safe concurrent access by multiple threads.
 *
 * @tparam T The type of elements stored in the queue.
 */
template <typename T> struct ts_queue {
  std::queue<T> internal;
  std::mutex access_mutex;
  std::condition_variable cond;

  /**
   * @brief Push a value onto the queue.
   *
   * This method adds a new element to the queue and notifies the waiting
   * threads.
   *
   * @param value The value to be added to the queue.
   */
  void push(const T &value);

  /**
   * @brief Pop a value from the queue.
   *
   * This method removes and returns the front element from the queue. If the
   * queue is empty, the method will block until an element is available.
   *
   * @return An optional containing the popped value, or std::nullopt if the
   * queue is empty.
   */
  std::optional<T> pop();

  /**
   * @brief Check if the queue is empty.
   *
   * This method checks whether the queue is empty without blocking.
   *
   * @return True if the queue is empty, false otherwise.
   */
  bool empty();

  /**
   * @brief Destructor for the ts_queue struct.
   * The destructor notifies all waiting threads, allowing them to wake up and
   * check for termination conditions.
   */
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
