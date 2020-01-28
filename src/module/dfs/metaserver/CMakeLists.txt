aux_source_directory(. SRC_LIST) # 搜索当前目录下的所有.cpp文件


# add_library(libbase STATIC IMPORTED)
# set_target_properties(libbase PROPERTIES IMPORTED_LOCATION ${MODULE_DFS_DIR}/libbase.a)
add_executable(metaserver ${SRC_LIST})
target_link_libraries(
    metaserver
    libbase 
    util  
    message 
    pthread
)

install(TARGETS metaserver DESTINATION bin)
