add_executable(
  new_concurrent_test
  LockTest.cpp
  $<TARGET_OBJECTS:obj_base>
)

target_link_libraries(
  new_concurrent_test
  gtest
  gtest_main
  pthread
) 

install(TARGETS new_concurrent_test DESTINATION tests/base)