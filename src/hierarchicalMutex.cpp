#include <iostream>
#include <mutex>
#include <climits>
#include <stdexcept>
using namespace std;

class hierarchicalMutex {
  std::mutex internal_mutex;
  const int hierarchy_value;
  int previos_hierarchy_value;
  static thread_local int this_thread_hierarchy_value;

  void check_for_hierarchy_violation() {
    if (this_thread_hierarchy_value <= hierarchy_value)
      throw std::logic_error("Mutex Hierarchy Violated\n");
  }

  void update_hierarchy_value() {
    previos_hierarchy_value = this_thread_hierarchy_value;
    this_thread_hierarchy_value = hierarchy_value;
  }

 public:
  explicit hierarchicalMutex(int value) : hierarchy_value(value), previos_hierarchy_value(0) {}

  void lock() {
    check_for_hierarchy_violation();
    internal_mutex.lock();
    update_hierarchy_value();
  }

  void unlock() {
    if (this_thread_hierarchy_value != hierarchy_value)
      throw std::logic_error("Mutex Hierarch Violated\n");

    this_thread_hierarchy_value = previos_hierarchy_value;
    internal_mutex.unlock();
  }

  bool try_lock() {
    check_for_hierarchy_violation();
    if (!internal_mutex.try_lock()) return false;

    update_hierarchy_value();
    return true;
  }
};

thread_local int hierarchicalMutex::this_thread_hierarchy_value(INT_MAX);

hierarchicalMutex high(10000); 
hierarchicalMutex low(5000);  

int main() {
  try {
    lock_guard<hierarchicalMutex> l1(high);
    lock_guard<hierarchicalMutex> l2(low);
    cout << "OK: locked high then low\n";
  } catch (const logic_error& e) {
    cout << "Unexpected: " << e.what();
  }

  try {
    lock_guard<hierarchicalMutex> l1(low);
    lock_guard<hierarchicalMutex> l2(high);  // throws
    cout << "Should not reach here\n";
  } catch (const logic_error& e) {
    cout << "Caught: " << e.what();
  }

  return 0;
}