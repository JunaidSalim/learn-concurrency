#include <iostream>
#include <future>
#include <thread>
#include <chrono>
#include <atomic>

using namespace std;

int find_answer() {
  this_thread::sleep_for(chrono::milliseconds(5000));
  return 42;
}

void do_something() { cout << "do_something called" << endl; }

int main() {
  std::future<int> answer = std::async(find_answer);
  do_something();

  while (answer.wait_for(chrono::milliseconds(1000)) != future_status::ready) {
    cout << "Still working...\n";
  }

  cout << "The answer is: " << answer.get() << endl;

  return 0;
}