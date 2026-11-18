#include <condition_variable>
#include <mutex>
#include <chrono>
#include <iostream>
#include <thread>
std::condition_variable cv;
bool done;
std::mutex m;

int main() {
  auto time_s = std::chrono::steady_clock::now();
  auto const timeout = time_s + std::chrono::milliseconds(5000);
  std::unique_lock<std::mutex> lk(m);
  while (true) {
    if (cv.wait_until(lk, timeout) == std::cv_status::timeout) {
      break;
    }
  }
  auto time_e = std::chrono::steady_clock::now();
  auto time_p = time_e - time_s;
  std::cout << "Elapsed: "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(time_p).count() / 1e9 << " s"
            << std::endl;

  return 0;
}