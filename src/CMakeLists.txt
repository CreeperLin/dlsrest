cmake_minimum_required(VERSION 3.9)
project(dlsrest-src)

find_package(cpprestsdk REQUIRED)
add_library(dlsrest common/utils.cpp)
add_library(dlsrest-impl impl/server_impl_rest.cpp impl/client_impl_rest.cpp)
add_library(dlsrest-server server/server.cpp)
add_library(dlsrest-client client/client.cpp)
add_executable(server bin/server.cpp)
target_link_libraries(dlsrest-impl cpprestsdk::cpprest)
target_link_libraries(dlsrest-server dlsrest dlsrest-impl)
target_link_libraries(dlsrest-client dlsrest dlsrest-impl)
target_link_libraries(server PRIVATE dlsrest-server)
