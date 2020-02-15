cmake_minimum_required (VERSION 2.8.11)

add_compile_options(-std=c++11)

set(CXX_FLAGS
  -Wno-deprecated
  -Wliteral-suffix
  -Wunused-result
  -std=c++11
)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

add_subdirectory (base)
# add_subdirectory (module)

set(NEPTUNE_SRC_HOME ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(AFTER ${NEPTUNE_SRC_HOME}/base)
# include_directories(AFTER ${NEPTUNE_SRC_HOME}/module)

