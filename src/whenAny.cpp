#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "experimentalFuture.h"
#include "tools.h"
using namespace std;
using namespace std::chrono;

int slow_task(int value, int delay_ms) {
  this_thread::sleep_for(milliseconds(delay_ms));
  cout << "[" << tools::ms_since_start() << " ms] task " << value << " finished\n";
  return value;
}

int main() {
  cout << "--- Test 1: three tasks (300, 100, 200 ms), wait for all ---\n";
  vector<ts::future<int>> results;
  results.push_back(ts::spawn_async([] { return slow_task(1, 300); }));
  results.push_back(ts::spawn_async([] { return slow_task(2, 100); }));
  results.push_back(ts::spawn_async([] { return slow_task(3, 200); }));

  auto any = ts::when_any(results.begin(), results.end());
  cout << "[" << tools::ms_since_start() << " ms] when_all returned immediately; original futures valid? "
       << results[0].valid() << " (0 = moved out)\n";

  auto ready = any.get();
  cout << "[" << tools::ms_since_start() << " ms] all ready, sum = ";
  int sum = 0;
  sum += ready.get();
  cout << sum << " (expected 6)\n";
}
