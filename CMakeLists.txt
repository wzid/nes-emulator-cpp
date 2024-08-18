cmake_minimum_required(VERSION 3.16)
project(nes_emulator_cpp)

set(CMAKE_CXX_STANDARD 20) # C++ 20
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

add_executable(${PROJECT_NAME} src/main.cpp)

# Variables storing SDL framework locations
set(SDL2
  /Library/Frameworks/SDL2.framework)
set(SDL2_image
  /Library/Frameworks/SDL2_image.framework)
set(SDL2_ttf
  /Library/Frameworks/SDL2_ttf.framework)

target_link_libraries(
  ${PROJECT_NAME} PRIVATE
  cpu_lib
  ${SDL2}/Versions/A/SDL2
  ${SDL2_image}/Versions/A/SDL2_image
  ${SDL2_ttf}/Versions/A/SDL2_ttf
)

target_include_directories(
  ${PROJECT_NAME} PRIVATE
  src/cpu
  ${SDL2}/Versions/A/Headers
  ${SDL2_image}/Versions/A/Headers
  ${SDL2_ttf}/Versions/A/Headers
)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(json_test src/main-json-test.cpp)
target_include_directories(json_test PRIVATE src/cpu)
target_link_libraries(json_test PRIVATE cpu_lib nlohmann_json::nlohmann_json)