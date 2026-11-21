#ifndef TOOLS_H
#define TOOLS_H

// Small helpers shared by the example programs in src/.
// Everything is `inline`, so including this header in several files is fine (C++17).

#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace tools {

// The moment the program started (the first time this header's variables are initialized).
inline const auto start_time = std::chrono::steady_clock::now();

// Guards std::cout so lines printed by different threads don't get mixed up.
inline std::mutex log_mutex;

// Milliseconds since the program started, with the fractional part (e.g. 100.382).
inline double ms_since_start() {
  return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start_time)
      .count();
}

// Thread-safe, timestamped line:  [123.4 ms] message
inline void log(const std::string& what) {
  std::ostringstream line;  // format into a local stream so std::cout's own settings stay untouched
  line << "[" << std::fixed << std::setprecision(1) << ms_since_start() << " ms] " << what << '\n';
  std::lock_guard<std::mutex> lk(log_mutex);
  std::cout << line.str() << std::flush;
}

// Same, with a name in front:  [123.4 ms] who: message
inline void log(const std::string& who, const std::string& what) { log(who + ": " + what); }

}  // namespace tools

#endif
