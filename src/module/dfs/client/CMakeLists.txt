aux_source_directory(. SRC_LIST) # 搜索当前目录下的所有.cpp文件
list(REMOVE_ITEM SRC_LIST "./dfs_unique_store.cpp")
list(REMOVE_ITEM SRC_LIST "./tair_cache_helper.cpp")
list(REMOVE_ITEM SRC_LIST "./tair_unique_handler.cpp")
add_library(client ${SRC_LIST})
target_link_libraries(
    client
    libbase 
    util  
    message 
    pthread
)