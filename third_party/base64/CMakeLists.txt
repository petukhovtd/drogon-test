project(cpp-base64)

add_library(${PROJECT_NAME} cpp-base64/base64.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC .)
