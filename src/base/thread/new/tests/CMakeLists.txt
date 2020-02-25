add_executable(
  new_thread_test
  ThreadIdNameManagerTest.cpp
)

target_link_libraries(
  new_thread_test
  gtest
  gtest_main
  pthread
) 

install(TARGETS new_thread_test DESTINATION tests/base)