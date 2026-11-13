#include "threadSafeQueue.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
using namespace std;

int failures = 0;
void check(const char* name, bool ok) {
  cout << (ok ? "PASS  " : "FAIL  ") << name << endl;
  if (!ok) failures++;
}

int main() {
  threadSafeQueue<int> q;

  check("new queue is empty", q.empty());
  q.push(1);
  q.push(2);
  q.push(3);
  check("not empty after push", !q.empty());
  q.print();

  threadSafeQueue<int> copy(q);
  copy.print();
  int v = 0;
  copy.try_pop(v);
  check("copy pop doesn't affect original", v == 1 && !q.empty());
  q.print();

  check("try_pop(T&) returns true", q.try_pop(v));
  check("try_pop(T&) gets 1 (FIFO)", v == 1);

  auto p = q.try_pop();
  check("try_pop() gets 2", p && *p == 2);

  q.wait_and_pop(v);
  check("wait_and_pop(T&) gets 3", v == 3);

  check("queue empty again", q.empty());
  check("try_pop(T&) on empty is false", !q.try_pop(v));
  check("try_pop() on empty is null", q.try_pop() == nullptr);

  thread producer([&] {
    this_thread::sleep_for(chrono::milliseconds(300));
    q.push(42);
  });
  auto start = chrono::steady_clock::now();
  auto r = q.wait_and_pop();  // shared_ptr version, blocks here
  auto ms =
      chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count();
  producer.join();
  check("wait_and_pop() got 42", r && *r == 42);
  check("wait_and_pop() actually waited (~300ms)", ms >= 250);

  threadSafeQueue<int> work;
  const int producers = 4, per_producer = 250;
  atomic<int> consumed{0};
  atomic<long> sum{0};
  vector<thread> ts;
  for (int c = 0; c < 4; c++)
    ts.emplace_back([&] {
      int x;
      while (true) {
        work.wait_and_pop(x);
        if (x < 0) break;  // sentinel
        sum += x;
        consumed++;
      }
    });
  vector<thread> ps;
  for (int p2 = 0; p2 < producers; p2++)
    ps.emplace_back([&] {
      for (int i = 1; i <= per_producer; i++) work.push(i);
    });
  for (auto& t : ps) t.join();
  for (int c = 0; c < 4; c++) work.push(-1);  // one stop signal per consumer
  for (auto& t : ts) t.join();
  check("4 producers x 250 items all consumed", consumed == producers * per_producer);
  check("sum is correct (no loss/duplicates)",
        sum == producers * (per_producer * (per_producer + 1) / 2));
  check("work queue empty at the end", work.empty());

  cout << (failures == 0 ? "ALL PASSED" : "SOME FAILED") << endl;
  return failures;
}