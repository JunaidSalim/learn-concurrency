#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
using namespace std;
using namespace std::chrono;

const auto t0 = steady_clock::now();
mutex log_mutex;
void log(const string& who, const string& what) {
  lock_guard<mutex> lk(log_mutex);
  cout << "[" << duration_cast<milliseconds>(steady_clock::now() - t0).count() << " ms] " << who
       << ": " << what << endl;
}

mutex mut;
queue<int> data_queue;
condition_variable data_cond;

const int LAST = -1;

void data_preparation_thread() {
  for (int i = 1; i <= 5; i++) {
    {
      lock_guard<mutex> lk(mut);
      data_queue.push(i);
      log("producer", "pushed " + to_string(i));
    }
    data_cond.notify_one();
    log("producer", "notified");
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
    log("consumer", "checking queue (empty = " + to_string(data_queue.empty()) + ")");

    data_cond.wait(lk, [] { return !data_queue.empty(); });

    int data = data_queue.front();
    data_queue.pop();
    lk.unlock();
    if (data == LAST) {
      log("consumer", "got last item, stopping");
      break;
    }
    log("consumer", "processing " + to_string(data));
  }
}

int main() {
  thread consumer(data_processing_thread);  // starts first: queue is empty, so it must wait
  thread producer(data_preparation_thread);
  producer.join();
  consumer.join();
  return 0;
}

void TransmissionLoop() {
  while(1) {
    // 1. mutex hold (loc)
    // 2. if false, loc release, sleep
    // 3. notified
      data_cond.wait(loc, [] { return received; });

    if (received) {
      received = false;
      SendMessage();
    } else {
      continue;
    }
  }
}

void ReceiveLoop() {
  while(1) {
    socket.Receive();
    unique_lock<mutex> lk(mut);
    data_cond.notify_one();
    received = true;
  }
}

