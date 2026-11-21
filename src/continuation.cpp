#include <chrono>
#include <future>
#include <iostream>
#include <thread>
using namespace std;

template <typename T, typename F>
auto then(future<T> f, F func) {
  return async(launch::async, [f = std::move(f), func]() mutable { return func(std::move(f)); });
}

int find_the_answer() {
  this_thread::sleep_for(chrono::seconds(3));
  return 42;
}

void find_the_question(future<int> f) { cout << "Received this value: " << f.get() << endl; }

int main() {
  future<int> answer = async(launch::async, find_the_answer);
  cout << "main: free to do other work while waiting\n";

  auto fut2 = then(std::move(answer), find_the_question);  // like answer.then(find_the_question)

  fut2.get();  // wait for the continuation to finish
  return 0;
}
