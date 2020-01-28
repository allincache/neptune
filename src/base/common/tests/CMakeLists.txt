add_executable(atomictest AtomicTest.cpp)

target_link_libraries(
    atomictest
    gtest
    pthread
) 

install(TARGETS atomictest DESTINATION tests/common)