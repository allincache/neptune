add_executable(
  common_test
  AtomicTest.cpp
  CDefineTest.cpp
  ConfigTest.cpp
  $<TARGET_OBJECTS:obj_base>
)

target_link_libraries(
  common_test
  gtest
  gtest_main
  pthread
) 

install(TARGETS common_test DESTINATION tests/base)