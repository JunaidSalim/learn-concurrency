#include <iostream>
#include <mutex>
#include <vector>
#include <thread>
#include <atomic>
using namespace std;

class threadSafeInitalizer {
 private:
  int connection_details;
  bool connection;
  std::once_flag connection_init_flag;

  void open_connection() {
    cout << "threadSafeInitalizer::open_connection called\n";
    connection = true;
  }

 public:
  threadSafeInitalizer(bool conn_ = false) : connection(conn_) {}

  void send_data(int data) {
    std::call_once(connection_init_flag, &threadSafeInitalizer::open_connection, this);
    // cout << "Sending Data: " << data << endl;
  }
  void receive_data(int data) {
    std::call_once(connection_init_flag, &threadSafeInitalizer::open_connection, this);
    // cout << "Receiving Data: " << data << endl;
  }
};

class threadInitalizer {
  bool connection = false;  // must be initialized, otherwise it holds a garbage value
  std::mutex init_mutex;
  void open_connection() {
    cout << "threadInitalizer::open_connection called\n";
    connection = true;
  }
  void ensure_open() {
    std::lock_guard<std::mutex> lk(init_mutex);
    if (!connection) open_connection();
  }

 public:
  void send_data(int data) {
    ensure_open();
    // cout << "Sending Data: " << data << endl;
  }
  void receive_data(int data) {
    ensure_open();
    // cout << "Receiving Data: " << data << endl;
  }
};

template <class T>
double time_ms(T& obj, int nthreads, int calls) {
  obj.send_data(0);  // initialize first: time only the "already initialized" path
  atomic<bool> go{false};
  vector<thread> ts;
  for (int t = 0; t < nthreads; ++t)
    ts.emplace_back([&] {
      while (!go) {
      }  // wait for the start signal
      for (int i = 0; i < calls; ++i) obj.send_data(i);
    });
  auto start = chrono::steady_clock::now();
  go = true;  // start the clock and the threads together
  for (auto& t : ts) t.join();
  return chrono::duration<double, milli>(chrono::steady_clock::now() - start).count();
}

int main() {
  threadSafeInitalizer a;
  threadInitalizer b;
  cout << "call_once: " << time_ms(a, 8, 1'000'000) << " ms\n";
  cout << "mutex    : " << time_ms(b, 8, 1'000'000) << " ms\n";
}