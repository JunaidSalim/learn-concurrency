#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "experimentalFuture.h"
using namespace std;

int find_the_answer() {
  this_thread::sleep_for(chrono::seconds(1));
  return 42;
}
string find_the_question(ts::future<int> f) { return "What is " + to_string(f.get()) + "?"; }
void show(ts::future<string> f) { cout << "Received: " << f.get() << endl; }

int main() {
  auto done = ts::spawn_async(find_the_answer).then(find_the_question).then(show);  // chain of 3
  cout << "main: free to work while the chain runs\n";
  done.get();  // wait for the last step
}
