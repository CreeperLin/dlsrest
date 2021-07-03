cmake_minimum_required(VERSION 3.9)
project(dlsrest-test)

find_package(cpprestsdk REQUIRED)
include_directories (${DLS_SOURCE_DIR}/src)
add_executable(test-lock test_lock.cpp)
add_executable(test-stress test_stress.cpp)
target_link_libraries(test-lock PRIVATE dlsrest-client)
target_link_libraries(test-stress PRIVATE dlsrest-client)
