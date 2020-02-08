add_executable(
  log_test
  LogTest.cpp
  $<TARGET_OBJECTS:obj_base>
)

target_link_libraries(
  log_test
  pthread
) 

install(TARGETS log_test DESTINATION tests/base)