cmake_minimum_required(VERSION 3.10)
project(learn-concurrency CXX)

set(CMAKE_CXX_STANDARD 17)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

enable_testing()

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp)

foreach(src ${SOURCES})
  get_filename_component(name ${src} NAME_WE)
  add_executable(${name} ${src})
  target_include_directories(${name} PRIVATE include)
  target_link_libraries(${name} PRIVATE Threads::Threads)
endforeach()

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS test/*.cpp)
add_executable(unit_tests ${TEST_SOURCES})
target_include_directories(unit_tests PRIVATE include)
target_link_libraries(unit_tests PRIVATE GTest::gtest_main Threads::Threads)

include(GoogleTest)
gtest_discover_tests(unit_tests)
