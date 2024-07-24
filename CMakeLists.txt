cmake_minimum_required(VERSION 3.14)
project(nes_emulator_cpp)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_library(cpu_lib src/cpu/cpu.cpp)

add_executable(
  cpu_test
  test/cpu_test.cpp
)
target_include_directories(cpu_test PRIVATE src/cpu)

target_link_libraries(cpu_test cpu_lib GTest::gtest_main)


include(GoogleTest)
gtest_discover_tests(cpu_test)
