add_executable(
  new_time_test
  TimeTest.cpp
)

target_link_libraries(
  new_time_test
  gtest
  gtest_main
  pthread
) 

install(TARGETS new_time_test DESTINATION tests/base)