#include <iostream>
#include <future>
#include <list>
#include <algorithm>
#include <chrono>
#include <random>

/*
Results:

For NUM_ELEMENTS: 1e3
Sequential Int Time: 0.0508585
Parallel Int Time: 0.0362925

Sequential Double Time: 0.057828
Parallel Double Time: 0.0335001


For NUM_ELEMENTS: 1e5
Sequential Int Time: 0.0508394
Parallel Int Time: 0.0317059

Sequential Double Time: 0.62099
Parallel Double Time: 0.424027

For NUM_ELEMENTS: 1e6
Sequential Int Time: 6.21081
Parallel Int Time: 3.07299

Sequential Double Time: 6.89977
Parallel Double Time: 4.34007
*/

#define NUM_ELEMENTS 1e6

template <typename T>
std::list<T> sequential_quick_sort(std::list<T> input) {
  if (input.empty()) return input;

  std::list<T> result;
  result.splice(result.begin(), input, input.begin());
  T const& pivot = *result.begin();

  auto divide_point =
      std::partition(input.begin(), input.end(), [&](T const& t) { return t < pivot; });

  std::list<T> lower_part;
  lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

  auto new_lower(sequential_quick_sort(std::move(lower_part)));
  auto new_higher(sequential_quick_sort(std::move(input)));
  result.splice(result.end(), new_higher);
  result.splice(result.begin(), new_lower);
  return result;
}

template <typename T>
std::list<T> parallel_quick_sort(std::list<T> input, int depth = 3) {
  if (input.empty()) return input;

  std::list<T> result;

  result.splice(result.begin(), input, input.begin());
  T const& pivot = *result.begin();

  auto divide_point =
      std::partition(input.begin(), input.end(), [&](T const& t) { return t < pivot; });

  std::list<T> lower_part;
  lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

  std::list<T> new_lower, new_higher;
  if (depth > 0) {
    std::future<std::list<T>> lower_future(
        std::async(std::launch::async, &parallel_quick_sort<T>, std::move(lower_part), depth - 1));
    new_higher = parallel_quick_sort(std::move(input), depth - 1);
    new_lower = lower_future.get();
  } else {
    new_lower = sequential_quick_sort(std::move(lower_part));
    new_higher = sequential_quick_sort(std::move(input));
  }

  result.splice(result.end(), new_higher);
  result.splice(result.begin(), new_lower);
  return result;
}

int main() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(0.0, 1000000.0);

  std::list<int> l1, l2;
  std::list<double> l3, l4;
  for (int i = 0; i < NUM_ELEMENTS; i++) {
    double rand_double = dist(gen);
    l3.push_back(rand_double);
    l4.push_back(rand_double);
  }

  for (int i = 0; i < NUM_ELEMENTS; i++) {
    int rand_int = dist(gen);
    l1.push_back(rand_int);
    l2.push_back(rand_int);
  }

  auto time_s = std::chrono::steady_clock::now();
  sequential_quick_sort<int>(l1);
  auto dur = std::chrono::steady_clock::now() - time_s;

  double time_seq_int = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count() / 1e9;

  time_s = std::chrono::steady_clock::now();
  parallel_quick_sort<int>(l2);
  dur = std::chrono::steady_clock::now() - time_s;

  double time_par_int = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count() / 1e9;

  time_s = std::chrono::steady_clock::now();
  sequential_quick_sort<double>(l3);
  dur = std::chrono::steady_clock::now() - time_s;

  double time_seq_doub = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count() / 1e9;

  time_s = std::chrono::steady_clock::now();
  parallel_quick_sort<double>(l4);
  dur = std::chrono::steady_clock::now() - time_s;

  double time_par_doub = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count() / 1e9;

  std::cout << "Sequential Int Time: " << time_seq_int << std::endl
            << "Parallel Int Time: " << time_par_int << std::endl
            << std::endl
            << "Sequential Double Time: " << time_seq_doub << std::endl
            << "Parallel Double Time: " << time_par_doub << std::endl;

  return 0;
}
