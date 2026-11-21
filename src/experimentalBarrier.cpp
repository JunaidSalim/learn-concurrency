#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "experimentalBarrier.h"
using namespace std;
using namespace std::chrono;

const auto t0 = steady_clock::now();
mutex print_mutex;
void log(const string& what) {
  lock_guard<mutex> lk(print_mutex);
  cout << "[" << fixed << setprecision(0)
       << duration<double, milli>(steady_clock::now() - t0).count() << " ms] " << what << endl;
}

void worker(int id, int rounds, ts::barrier& sync, bool leaves_after_round_1 = false) {
  for (int round = 1; round <= rounds; round++) {
    this_thread::sleep_for(milliseconds(100 * id));  // thread id needs id*100 ms per round
    log("worker " + to_string(id) + ": finished round " + to_string(round));

    if (leaves_after_round_1 && round == 1) {
      sync.arrive_and_drop();
      log("worker " + to_string(id) + ": dropped out");
      return;
    }
    sync.arrive_and_wait();  // nobody starts the next round until all have finished this one
  }
}

int main() {
  // Test 1: 3 workers, 2 rounds. The barrier is reused for round 2 without any reset.
  log("--- Test 1: 3 workers, 2 rounds ---");
  {
    ts::barrier sync(3);
    vector<thread> threads;
    for (int id = 1; id <= 3; id++) threads.emplace_back(worker, id, 2, ref(sync), false);
    for (auto& t : threads) t.join();
  }

  // Test 2: worker 3 leaves after round 1, so rounds 2 and 3 only wait for 2 workers.
  log("--- Test 2: worker 3 drops out after round 1 ---");
  {
    ts::barrier sync(3);
    vector<thread> threads;
    for (int id = 1; id <= 3; id++) threads.emplace_back(worker, id, 3, ref(sync), id == 3);
    for (auto& t : threads) t.join();
  }
  log("done");
}
