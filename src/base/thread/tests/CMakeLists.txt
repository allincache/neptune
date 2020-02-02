add_executable(
  thread_test
  CThreadTest.cpp
  $<TARGET_OBJECTS:obj_base>
)

target_link_libraries(
  thread_test
  pthread
) 

install(TARGETS thread_test DESTINATION tests/base)