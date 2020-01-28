set(BASE_HOME ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(AFTER ..)

file(GLOB_RECURSE BASE_SRC_FILES ${BASE_HOME}/*.cpp)

add_library(
   base STATIC ${BASE_SRC_FILES}
)

add_subdirectory(common)

install(TARGETS base DESTINATION lib)

#file(GLOB HEADERS ${BASE_HOME}/"*.h")
#install(FILES ${HEADERS} DESTINATION include)
