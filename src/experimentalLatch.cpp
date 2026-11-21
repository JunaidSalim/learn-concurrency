#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "experimentalLatch.h"
using namespace std;
using namespace std::chrono;

const auto t0 = steady_clock::now();
mutex print_mutex;

void runner(int id, ts::latch& start_gate) {
  {
    lock_guard<mutex> lk(print_mutex);
    cout << "runner " << id << ": at the start line\n";
  }
  start_gate.wait();
  {
    lock_guard<mutex> lk(print_mutex);
    cout << "runner " << id << ": GO at "
         << duration_cast<microseconds>(steady_clock::now() - t0).count() << " us\n";
  }
}

int main() {
  ts::latch start_gate(1);

  vector<thread> runners;
  for (int i = 1; i <= 3; i++) runners.emplace_back(runner, i, ref(start_gate));

  this_thread::sleep_for(milliseconds(500)); 
  cout << "main: ready, set, go!\n";
  start_gate.count_down();

  for (auto& r : runners) r.join();
}
