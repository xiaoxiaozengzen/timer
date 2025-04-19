cmake_minimum_required(VERSION 3.21)
project(timer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_library(timer ${SOURCES})
target_include_directories(
  timer PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

add_executable(timer_example example/example.cpp)
target_link_libraries(timer_example PRIVATE timer)
target_include_directories(
  timer_example PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)