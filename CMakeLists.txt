cmake_minimum_required(VERSION 3.16)
project(drogon-test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

add_subdirectory(third_party)

enable_testing()
add_custom_target(build-test)

add_subdirectory(myapp)
