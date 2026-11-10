#include <iostream>
#include <thread>
#include <vector>
using namespace std;

void DoSomething(int id) { cout << "Id: " << id << endl; }

int main() {
  vector<thread> threads;

  for (int i = 0; i < 20; i++) {
    threads.emplace_back(DoSomething, i);
    cout << "Passing Id: " << i << endl;
  }

  for (auto& entry : threads) {
    entry.join();
  }

  cout << "Main thread finished" << endl;
  return 0;
}