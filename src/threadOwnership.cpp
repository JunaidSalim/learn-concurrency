#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

class scoped_thread {
 public:
  thread t;
  explicit scoped_thread(thread t_) : t(std::move(t_)) {
    if (!t.joinable()) {
      cout << "Error" << endl;
    }
    cout << "[Thread " << t.get_id() << "] " << endl;
  }

  ~scoped_thread() { t.join(); }
  scoped_thread(scoped_thread const&) = delete;
  scoped_thread& operator=(scoped_thread const&) = delete;
};

void DoSomething() {
  cout << "Function Called" << endl;
  cout << "[Thread " << this_thread::get_id() << "] " << endl;
}

void DoSomethingElse(int id) { cout << "Doing Task " << id << endl; }

int main() {
  scoped_thread t1{thread(DoSomething)};

  cout << "Main Thread finished" << endl;
  return 0;
}

// class MyThread {
//  public:
//   MyThread(int id_) { id = id_; }

//   MyThread(MyThread&& t) { id = t.id; }

//   int id;
// }

// class A {
//  public:
//   A(MyThread a_) : a(std::move(a_)) {}

//   void Print() { std::cout << "a = " << a << std::endl; }

//  private:
//   MyThread a;
// };

// int main() {
//   int c = 5;
//   A obj(c);
//   c = 6;
// }