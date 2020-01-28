aux_source_directory(. SRC_LIST) # 搜索当前目录下的所有.cpp文件
#list(REMOVE_ITEM SRC_LIST "./new_client.cpp")
#list(REMOVE_ITEM SRC_LIST "./base_packet.cpp")
add_library(util ${SRC_LIST})
target_link_libraries(util libbase pthread)