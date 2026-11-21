#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include "tools.h"
using namespace std;
using namespace std::chrono;

mutex mut;
queue<int> data_queue;
condition_variable data_cond;

const int LAST = -1;

void data_preparation_thread() {
  for (int i = 1; i <= 5; i++) {
    {
      lock_guard<mutex> lk(mut);
      data_queue.push(i);
      tools::log("producer", "pushed " + to_string(i));
    }
    data_cond.notify_one();
    tools::log("producer", "notified");
    this_thread::sleep_for(milliseconds(300));
  }
  {
    lock_guard<mutex> lk(mut);
    data_queue.push(LAST);
  }
  data_cond.notify_one();
}

void data_processing_thread() {
  while (true) {
    unique_lock<mutex> lk(mut);
    tools::log("consumer", "checking queue (empty = " + to_string(data_queue.empty()) + ")");

    data_cond.wait(lk, [] { return !data_queue.empty(); });

    int data = data_queue.front();
    data_queue.pop();
    lk.unlock();
    if (data == LAST) {
      tools::log("consumer", "got last item, stopping");
      break;
    }
    tools::log("consumer", "processing " + to_string(data));
  }
}

int main() {
  thread consumer(data_processing_thread);  // starts first: queue is empty, so it must wait
  thread producer(data_preparation_thread);
  producer.join();
  consumer.join();
  return 0;
}