set(BASE_HOME ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(AFTER ..)

# file(GLOB_RECURSE BASE_SRC_FILES ${BASE_HOME}/*.cpp)

set (EXCLUDE_DIR "/tests/")
file (GLOB_RECURSE BASE_SRC_FILES "*.cpp")
foreach (TMP_PATH ${BASE_SRC_FILES})
  string (FIND ${TMP_PATH} ${EXCLUDE_DIR} EXCLUDE_DIR_FOUND)
  if (NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
    list (REMOVE_ITEM BASE_SRC_FILES ${TMP_PATH})
  endif ()
endforeach(TMP_PATH)


add_library(
   base STATIC ${BASE_SRC_FILES}
)

add_library(
   obj_base OBJECT ${BASE_SRC_FILES}
)

#add_subdirectory(common)
#add_subdirectory(concurrent)
add_subdirectory(thread)

install(TARGETS base DESTINATION lib)

#file(GLOB HEADERS ${BASE_HOME}/"*.h")
#install(FILES ${HEADERS} DESTINATION include)
