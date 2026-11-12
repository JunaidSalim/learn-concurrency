#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>
using namespace std;

shared_mutex m;

// Probe the mutex from ANOTHER thread (try_lock on a mutex you already own is undefined).
// Returns {could take shared lock, could take exclusive lock}.
pair<bool, bool> probe() {
  pair<bool, bool> r{false, false};
  thread t([&] {
    if (m.try_lock_shared()) { r.first = true; m.unlock_shared(); }
    if (m.try_lock())        { r.second = true; m.unlock(); }
  });
  t.join();
  return r;
}

void check(const char* name, bool ok) { cout << (ok ? "PASS  " : "FAIL  ") << name << endl; }

int main() {
  {
    auto p = probe();
    check("free: reader can lock", p.first);
    check("free: writer can lock", p.second);
  }
  {
    shared_lock<shared_mutex> reader(m);  // main thread is a reader
    auto p = probe();
    check("reader held: another reader CAN lock", p.first);
    check("reader held: writer is BLOCKED", !p.second);
  }
  {
    unique_lock<shared_mutex> writer(m);  // main thread is a writer
    auto p = probe();
    check("writer held: reader is BLOCKED", !p.first);
    check("writer held: another writer is BLOCKED", !p.second);
  }
  {
    auto p = probe();
    check("released: reader can lock again", p.first);
    check("released: writer can lock again", p.second);
  }
}
