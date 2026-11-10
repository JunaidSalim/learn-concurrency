#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
using namespace std;

void f(int id, string const& s) {
  // One file per id in the system temp dir (e.g. /tmp/thread_2.txt)
  filesystem::path path = ("thread_" + to_string(id) + ".txt");

  ofstream out(path, ios::app);
  if (!out) {
    cerr << "Could not open " << path << endl;
    return;
  }
  out << "Id: " << id << " string: " << s << endl;
  cout << "Wrote to " << path << endl;
}

void f_main() {
  char buffer[1024];
  snprintf(buffer, sizeof(buffer), "%s", "World");
  thread t2(f, 2, buffer);
  t2.detach();

  cout << "f_main Finished" << endl;
}

int main() {
  thread t(f_main);
  t.join();
  cout << "Main Thread finished" << endl;
  return 0;
}
