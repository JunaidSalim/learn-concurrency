#ifndef EXPERIMENTAL_BARRIER_H
#define EXPERIMENTAL_BARRIER_H

#include <condition_variable>
#include <mutex>
#include <stdexcept>

namespace ts {

class barrier {
 protected:
  std::mutex m_;
  std::condition_variable cv_;
  int expected_;        // how many threads must arrive in the current round
  int arrived_ = 0;     // how many have arrived so far in the current round
  int generation_ = 0;  // which round we are in (changes each time the barrier opens)

  virtual void open_() {
    arrived_ = 0;
    ++generation_;
    cv_.notify_all();
  }

 public:
  explicit barrier(int count) : expected_(count) {
    if (count <= 0) throw std::invalid_argument("barrier count must be positive");
  }
  barrier(const barrier&) = delete;
  barrier& operator=(const barrier&) = delete;
  virtual ~barrier() = default;

  void arrive_and_wait() {
    std::unique_lock<std::mutex> lk(m_);
    if (expected_ <= 0) throw std::logic_error("barrier has no threads left in its group");
    const int my_generation = generation_;
    if (++arrived_ == expected_) {
      open_();
    } else {
      cv_.wait(lk, [&] { return my_generation != generation_; });
    }
  }

  void arrive_and_drop() {
    std::lock_guard<std::mutex> lk(m_);
    if (expected_ <= 0) throw std::logic_error("barrier has no threads left in its group");
    --expected_;
    if (arrived_ == expected_ && arrived_ > 0) open_();
  }
};

struct no_completion {
  int operator()() const { return -1; }
};

template <typename Completion = no_completion>
class flex_barrier : public barrier {
  Completion completion_;

  void open_() override {
    int next = completion_();
    if (next >= 0) expected_ = next;
    barrier::open_();
  }

 public:
  explicit flex_barrier(int count, Completion completion = Completion())
      : barrier(count), completion_(std::move(completion)) {}
};

}  // namespace ts

#endif
