add_executable(
  concurrent_test
  CondTest.cpp
  $<TARGET_OBJECTS:obj_base>
)

target_link_libraries(
  concurrent_test
  pthread
) 

install(TARGETS concurrent_test DESTINATION tests/base)