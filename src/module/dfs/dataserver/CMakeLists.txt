aux_source_directory(. SRC_LIST) # 搜索当前目录下的所有.cpp文件
list(REMOVE_ITEM SRC_LIST "./service.cpp")
add_library(obj_dataserver OBJECT ${SRC_LIST})

#link_libraries(libdataserver)

add_executable(dataserver service.cpp $<TARGET_OBJECTS:obj_dataserver>)

target_link_libraries(
  dataserver
  libbase 
  util
  message
  client
  pthread
)

install(TARGETS dataserver DESTINATION bin)