#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

void DoSomething() { cout << "Function Called" << endl; }

void DoSomethingElse(int id) { cout << "Doing Task " << id << endl; }

int main() {
  thread t(DoSomething);
  t.join();
  int id_response;

  cout << "Enter Something: ";
  cin >> id_response;

  thread t2(DoSomethingElse, 200);
  if (id_response == 1) {
    t2.detach();
  } else {
    t2.join();
  }
  //   this_thread::sleep_for(chrono::milliseconds(100));
  cout << "Main Thread finished" << endl;
  return 0;
}
