# learn-concurrency

Small C++ examples and exercises from my reading of *C++ Concurrency in Action, 2nd Edition* by Anthony Williams.

Each file in `src/` is a standalone program on one topic (threads, mutexes, condition variables, futures). `include/` contains headers, `test/` has GoogleTest tests.

## Setup (Linux)

Requirements: a C++17 compiler, CMake 3.10+ and GoogleTest.

```bash
sudo apt install build-essential cmake libgtest-dev
```

Build everything:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Run an example (each program is named after its file in `src/`):

```bash
./path_to_executable
```

Run the tests:

```bash
ctest
```
