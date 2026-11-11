#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <thread>
#include <vector>
using namespace std;

template <typename Iterator, typename T>
struct accumulate_block {
  void operator()(Iterator first, Iterator last, T& result) {
    result = std::accumulate(first, last, result);

    // Build the whole message first, then write once so threads don't interleave
    ostringstream oss;
    oss << "[Thread " << this_thread::get_id() << "] ";
    if (first != last) {
      // last is one-past-the-end, so the final element is prev(last)
      oss << "chunk: " << *first << " to " << *prev(last) << " (" << distance(first, last)
          << " elements)";
    } else {
      oss << "chunk: empty";
    }
    oss << " | partial sum = " << result << "\n";
    cout << oss.str();
  }
};

template <typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
  const long length = std::distance(first, last);
  if (!length) {
    return init;
  }

  // config
  const int min_per_thread = 5;
  const int max_threads = (length + min_per_thread - 1) / min_per_thread;  // 2
  int hardware_threads = 50;                                               // 12
  const int num_threads = std::min(hardware_threads != 0 ? hardware_threads : 4, max_threads);  //
  const int block_size = length / num_threads;

  cout << "Total elements: " << length << ", threads: " << num_threads
       << ", block size: " << block_size << " (last block takes the remainder)\n";

  vector<T> results(num_threads);
  vector<thread> threads(num_threads - 1);

  Iterator block_start = first;

  for (int i = 0; i < num_threads - 1; i++) {
    Iterator block_end = block_start;
    std::advance(block_end, block_size);
    threads[i] =
        thread(accumulate_block<Iterator, T>(), block_start, block_end, std::ref(results[i]));
    block_start = block_end;
  }

  accumulate_block<Iterator, T>()(block_start, last, results[num_threads - 1]);

  for (auto& entry : threads) {
    entry.join();
  }

  cout << "All threads joined. Partial results: ";
  for (const auto& r : results) {
    cout << r << " ";
  }
  cout << "\n";

  return std::accumulate(results.begin(), results.end(), init);
};

int main() {
  for (int i = 0; i < 100; i++) {
    vector<int> vect;
    for (int i = 0; i < 1000; i++) {
      vect.push_back(i);
    }

    int total = parallel_accumulate(vect.begin(), vect.end(), 0);
    cout << "Total: " << total << endl;
    cout << "Main Thread finished" << endl;
  }
  return 0;
}
