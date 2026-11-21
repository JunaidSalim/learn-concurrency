#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "experimentalBarrier.h"
#include "tools.h"
using namespace std;
using namespace std::chrono;

int main() {
  const int workers = 3;
  int partial[workers] = {0, 0, 0};
  int round_no = 0;

  auto completion = [&]() -> int {
    ++round_no;
    int total = 0;
    for (int v : partial) total += v;
    tools::log("  [serial] round " + to_string(round_no) + " complete, total = " + to_string(total));
    return round_no == 2 ? 2 : -1;
  };
  ts::flex_barrier sync(workers, completion);

  auto worker = [&](int id, int rounds) {
    for (int round = 1; round <= rounds; round++) {
      this_thread::sleep_for(milliseconds(100 * id));
      partial[id - 1] = id * round;
      tools::log("worker " + to_string(id) + ": finished round " + to_string(round));
      sync.arrive_and_wait();
    }
  };

  vector<thread> threads;
  threads.emplace_back(worker, 1, 3);
  threads.emplace_back(worker, 2, 3);
  threads.emplace_back(worker, 3, 2); 
  for (auto& t : threads) t.join();
  tools::log("done");
}
