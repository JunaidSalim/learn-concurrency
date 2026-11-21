#ifndef EXPERIMENTAL_FUTURE_H
#define EXPERIMENTAL_FUTURE_H

#include <exception>
#include <future>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>
#include <iterator>

namespace ts {

template <typename T>
class future {
 public:
  future() = default;
  explicit future(std::future<T> f) : f_(std::move(f)) {}
  future(future&&) = default;
  future& operator=(future&&) = default;

  T get() { return f_.get(); }
  bool valid() const { return f_.valid(); }
  void wait() const { f_.wait(); }
  bool is_ready() const {
    return f_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
  }

  // future1 = Function1()
  // future1.then(Function2)      future2 <= promise = Function2(future1)

  template <typename F>
  auto then(F func){
    using R = std::invoke_result_t<F, future<T>>;
    std::promise<R> p;
    future<R> res(p.get_future());
    std::thread([p = std::move(p), self = std::move(*this), func]() mutable {
      try {
        if constexpr (std::is_void_v<R>) {
          func(std::move(self));
          p.set_value_at_thread_exit();
        } else {
          p.set_value_at_thread_exit(func(std::move(self)));
        }
      } catch (...) {
        p.set_exception_at_thread_exit(std::current_exception());
      }
    }).detach();
    return res;
  }

 private:
  std::future<T> f_;
};

template <typename Func>
auto spawn_async(Func&& func) {
  using R = decltype(std::declval<Func>()());
  std::promise<R> promise_of_func;
  future<R> future_of_func(promise_of_func.get_future());
  std::thread([p = std::move(promise_of_func), f = std::decay_t<Func>(func)]() mutable {
    try {
      if constexpr (std::is_void_v<R>) {
        f();
        p.set_value_at_thread_exit();
      } else {
        p.set_value_at_thread_exit(f());
      }
    } catch (...) {
      p.set_exception_at_thread_exit(std::current_exception());
    }
  }).detach();
  return future_of_func;
}

template <typename InputIt>
auto when_all(InputIt first, InputIt last) {
  using F = typename std::iterator_traits<InputIt>::value_type;
  std::vector<F> futures;
  for (; first != last; first++) futures.push_back(std::move(*first));

  std::promise<std::vector<F>> promise_of_vector;
  future<std::vector<F>> future_of_vector(promise_of_vector.get_future());

  std::thread([promise_of_vector = std::move(promise_of_vector), fs = std::move(futures)]() mutable {
    for (auto& f : fs) f.wait();
    promise_of_vector.set_value_at_thread_exit(std::move(fs));
  }).detach();
  return future_of_vector;
}

template <typename InputIt>
auto when_any(InputIt first, InputIt last) {
  using F = typename std::iterator_traits<InputIt>::value_type;
  auto when_any_promise = std::make_shared<std::promise<F>>();
  auto claimed = std::make_shared<std::atomic<bool>>(false);
  future<F> future_when_any(when_any_promise->get_future());

  for (; first != last; ++first) {
    std::thread([when_any_promise, claimed, f = std::move(*first)]() mutable {
      f.wait();
      if (!claimed->exchange(true)) {
        when_any_promise->set_value(std::move(f));
      }
    }).detach();
  }
  return future_when_any; // can get when_any promise is set
}

}  // namespace ts

#endif