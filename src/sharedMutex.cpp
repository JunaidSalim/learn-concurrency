#include <chrono>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <thread>
#include <vector>
using namespace std;
using namespace std::chrono;

const auto t0 = steady_clock::now();
mutex log_mutex;

// Debug print: timestamp (ms since start) + who + what
void log(const string& who, const string& what) {
  lock_guard<mutex> lk(log_mutex);
  cout << "[" << duration_cast<milliseconds>(steady_clock::now() - t0).count() << " ms] " << who
       << ": " << what << endl;
}

class DNS_cache {
  vector<int> entries;
  mutable shared_mutex entry_mutex;

 public:
  DNS_cache(int range) {
    for (int i = 0; i < range; i++) entries.emplace_back(i * 5);
  }

  int find_entry(const string& who, int domain) {
    log(who, "waiting for READ lock");
    shared_lock<shared_mutex> rL(entry_mutex);
    log(who, "got READ lock");
    this_thread::sleep_for(milliseconds(500));  // hold the lock so overlaps are visible
    int v = entries[domain];
    log(who, "releasing READ lock");
    return v;
  }

  void update_or_add_entry(const string& who, int domain, int dns_details) {
    log(who, "waiting for WRITE lock");
    unique_lock<shared_mutex> wL(entry_mutex);
    log(who, "got WRITE lock");
    this_thread::sleep_for(milliseconds(500));
    entries[domain] = dns_details;
    log(who, "releasing WRITE lock");
  }
};

int main() {
  DNS_cache d_cache(10);

  cout << "--- Two readers together: both should get the lock at once ---\n";
  {
    thread r1([&] { d_cache.find_entry("reader1", 0); });
    thread r2([&] { d_cache.find_entry("reader2", 1); });
    r1.join();
    r2.join();
  }

  cout << "--- Writer first, then a reader: reader should wait for the writer ---\n";
  {
    thread w([&] { d_cache.update_or_add_entry("writer", 0, 100); });
    this_thread::sleep_for(milliseconds(100));  // let the writer grab the lock first
    thread r3([&] { log("reader3", "read value = " + to_string(d_cache.find_entry("reader3", 0))); });
    w.join();
    r3.join();
  }
  return 0;
}
