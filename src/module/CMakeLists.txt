cmake_minimum_required (VERSION 2.8.11)
project (neptune)

add_compile_options(-std=c++11)

SET(MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(MODULE_DFS_CONF_DIR /opt/neptune/dfs/conf)

include_directories(AFTER ./)
include_directories(AFTER ../)

#LINK_DIRECTORIES(${MODULE_DFS_DIR}/client)

FIND_LIBRARY(BASE libbase.a ../../build/release/lib/)

add_library(libbase STATIC IMPORTED)
set_target_properties(libbase PROPERTIES IMPORTED_LOCATION ${BASE})

add_subdirectory (dfs)

file(GLOB CONF_FILE_LIST "${MODULE_DIR}/dfs/conf/*.conf")
file(COPY ${CONF_FILE_LIST} DESTINATION ${MODULE_DFS_CONF_DIR})
