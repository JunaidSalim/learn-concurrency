#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>

template <typename T>
class threadSafeQueue {
 public:
  threadSafeQueue() {}
  threadSafeQueue(threadSafeQueue const& other) {
    std::lock_guard<std::mutex> lk(other.mut);
    data_queue = other.data_queue;
  }

  void push(T new_value) {
    std::lock_guard<std::mutex> lk(mut);
    data_queue.push(new_value);
    data_cond.notify_one();
  }

  void wait_and_pop(T& value) {
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk, [this] { return !data_queue.empty(); });
    value = data_queue.front();
    data_queue.pop();
  }

  std::shared_ptr<T> wait_and_pop() {
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk, [this] { return !data_queue.empty(); });
    std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
    data_queue.pop();
    return res;
  }

  bool try_pop(T& value) {
    std::lock_guard<std::mutex> lk(mut);
    if (data_queue.empty()) return false;

    value = data_queue.front();
    data_queue.pop();
    return true;
  }

  std::shared_ptr<T> try_pop() {
    std::lock_guard<std::mutex> lk(mut);
    if (data_queue.empty()) return std::shared_ptr<T>();
    std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
    data_queue.pop();
    return res;
  }

  bool empty() {
    std::lock_guard<std::mutex> lk(mut);
    return data_queue.empty();
  }

  void print() {
    std::lock_guard<std::mutex> lk(mut);

    std::queue<T> copy = data_queue;
    std::cout << "Queue" << " (front first): ";
    while (!copy.empty()) {
      std::cout << copy.front() << " ";
      copy.pop();
    }
    std::cout << std::endl;
  }

 private:
  mutable std::mutex mut;  // mutable: the copy constructor locks a const `other`
  std::queue<T> data_queue;
  std::condition_variable data_cond;
};

#endif