aux_source_directory(. SRC_LIST) 
add_executable(metaserver ${SRC_LIST})
target_link_libraries(
    metaserver
    base
    util  
    message 
    pthread
)

install(TARGETS metaserver DESTINATION bin)
