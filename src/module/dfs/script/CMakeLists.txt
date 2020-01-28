add_executable(
  clear_file_system
  clear_file_system.cpp
  $<TARGET_OBJECTS:obj_dataserver>
)

add_executable(
  format_file_system
  format_file_system.cpp
  $<TARGET_OBJECTS:obj_dataserver>
)

target_link_libraries(
    clear_file_system
    libbase 
    util
    message
    client
    pthread
)

target_link_libraries(
    format_file_system
    libbase 
    util
    message
    client
    pthread
)

SET(MODULE_DFS_SCRIPT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS clear_file_system DESTINATION script)
install(TARGETS format_file_system DESTINATION script)
install(FILES ${MODULE_DFS_SCRIPT_DIR}/disk.sh DESTINATION script)
