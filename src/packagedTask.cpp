#include <iostream>
#include <atomic>
#include <chrono>
#include <deque>
#include <future>
#include <thread>
#include <utility>
#include <mutex>
using namespace std;

std::mutex m;
std::deque<std::packaged_task<void()>> tasks;

std::atomic<bool> shutdown_requested{false};

bool gui_shutdown_message_received() { return shutdown_requested; }
void get_and_process_gui_message() { this_thread::sleep_for(chrono::milliseconds(1000)); }

void gui_thread() {
  while (true) {
    get_and_process_gui_message();
    if (gui_shutdown_message_received()) break;
    std::packaged_task<void()> task;
    {
      std::lock_guard<std::mutex> lk(m);
      if (tasks.empty()) {
        continue;
      }
      task = std::move(tasks.front());
      tasks.pop_front();
    }
    task();
  }
}

template <typename Func>
std::future<void> post_task_for_gui_thread(Func f) {
  std::packaged_task<void()> task(f);
  std::future<void> res = task.get_future();
  std::lock_guard<std::mutex> lk(m);
  tasks.push_back(std::move(task));
  return res;
}

int main() {
  std::thread gui_bg_thread(gui_thread);
  cout << "main thread:  " << this_thread::get_id() << endl;

  // Post two tasks; they run on the GUI thread, not here
  auto f1 =
      post_task_for_gui_thread([] { cout << "task 1 ran on " << this_thread::get_id() << endl; });
  auto f2 =
      post_task_for_gui_thread([] { cout << "task 2 ran on " << this_thread::get_id() << endl; });

  f1.get();
  f2.get();
  cout << "both tasks done" << endl;

  shutdown_requested = true;
  gui_bg_thread.join();
  return 0;
}