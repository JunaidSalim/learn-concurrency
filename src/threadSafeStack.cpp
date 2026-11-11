#include <iostream>
#include <memory>
#include <exception>
#include <stack>
#include <mutex>
using namespace std;

struct empty_stack : std::exception {
  const char* what() const noexcept override { return "Error:Empty Stack\n"; }
};

template <typename T>
class threadSafeStack {
 private:
  std::stack<T> data;
  mutable std::mutex m;

 public:
  threadSafeStack() {}
  threadSafeStack(const threadSafeStack& other) {
    std::lock_guard<std::mutex> lock(other.m);
    data = other.data;
  }

  threadSafeStack& operator=(const threadSafeStack&) = delete;

  void push(T new_value) {
    std::lock_guard<std::mutex> lock(m);
    data.push(std::move(new_value));
  }

  std::shared_ptr<T> pop() {
    std::lock_guard<std::mutex> lock(m);
    if (data.empty()) throw empty_stack();

    std::shared_ptr<T> const res(std::make_shared<T>(data.top()));
    data.pop();
    return res;
  }

  void pop(T& value) {
    std::lock_guard<std::mutex> lock(m);
    if (data.empty()) throw empty_stack();
    value = data.top();
    data.pop();
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(m);
    return data.empty();
  }

  void print(const string s) {
    std::lock_guard<std::mutex> lock(m);
    if (data.empty()) throw empty_stack();

    std::stack<T> copy = data;
    cout << s << " Stack Entries (top first): ";
    while (!copy.empty()) {
      cout << copy.top() << " ";
      copy.pop();
    }
    cout << endl;
  }
};

int main() {
  threadSafeStack<int> st;

  st.push(1);
  st.push(2);
  st.push(3);
  st.push(4);
  st.push(5);
  st.print("st");

  threadSafeStack<int> st2(st);
  st2.print("st2");

  st.pop();
  st.print("st");
  st2.print("st2");

  st.pop();
  st.pop();
  cout << "Is Empty " << st.empty() << endl;
  st.pop();
  st.pop();

  cout << "Is Empty " << st.empty() << endl;

  return 0;
}