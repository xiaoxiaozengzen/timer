add_executable(timer_example example.cpp)
target_link_libraries(timer_example PRIVATE timer pthread)
target_include_directories(
  timer_example PRIVATE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)