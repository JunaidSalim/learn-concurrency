#include <iostream>
#include <chrono>
#include <thread>

void do_something() {
  std::this_thread::sleep_for(std::chrono::seconds(2));
  std::cout << "do_something runned" << std::endl;
}

int main() {
  auto start = std::chrono::high_resolution_clock::now();
  do_something();
  auto stop = std::chrono::high_resolution_clock::now();
  std::cout << "do_something() took " << std::chrono::duration<double>(stop - start).count()
            << " seconds" << std::endl;
  return 0;
}