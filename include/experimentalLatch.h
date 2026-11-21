#ifndef EXPERIMENTAL_LATCH_H
#define EXPERIMENTAL_LATCH_H

#include <condition_variable>
#include <mutex>
#include <stdexcept>

namespace ts {

class latch {
  mutable std::mutex m_;
  std::condition_variable cv_;
  int count_;

 public:
  explicit latch(int count) : count_(count) {
    if (count < 0) throw std::invalid_argument("latch count must not be negative");
  }

  latch(const latch&) = delete;
  latch& operator=(const latch&) = delete;

  void count_down(int n = 1) {
    std::lock_guard<std::mutex> lk(m_);
    if (n < 0 || n > count_) throw std::logic_error("latch counted down below zero");
    count_ -= n;
    if (count_ == 0) cv_.notify_all();
  }

  bool is_ready() const {
    std::lock_guard<std::mutex> lk(m_);
    return count_ == 0;
  }

  void wait() {
    std::unique_lock<std::mutex> lk(m_);
    cv_.wait(lk, [this] { return count_ == 0; });
  }

  void count_down_and_wait() {
    std::unique_lock<std::mutex> lk(m_);
    if (count_ <= 0) throw std::logic_error("latch counted down below zero");
    if (--count_ == 0) {
      cv_.notify_all();
    } else {
      cv_.wait(lk, [this] { return count_ == 0; });
    }
  }
};

}  // namespace ts

#endif
