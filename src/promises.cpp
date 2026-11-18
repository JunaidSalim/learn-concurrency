#include <chrono>
#include <future>
#include <iostream>
#include <thread>
using namespace std;

void Do_Something(future<int> incoming, promise<bool> sent) {
  cout << "[Do_Something] waiting for data\n";
  int data = incoming.get();
  cout << "[Do_Something] got " << data << ", replying\n";
  sent.set_value(true);
}

int main() {
  promise<int> incoming;
  promise<bool> sent;
  future<bool> ack = sent.get_future();

  thread t(Do_Something, incoming.get_future(), std::move(sent));

  this_thread::sleep_for(chrono::seconds(1));
  cout << "[main] data arrives\n";
  incoming.set_value(10);

  bool ok = ack.get();
  cout << "[main] reply sent? " << ok << "\n";
  t.join();
}
