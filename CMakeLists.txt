cmake_minimum_required(VERSION 3.16)
project(drogon-test)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

add_subdirectory(third_party/drogon)

set(MYAPP_SOURCE 
    myapp/user.cc 
    myapp/user_db.cc
)

add_executable(${PROJECT_NAME} main.cpp ${MYAPP_SOURCE} api_v1_User.cc)
target_include_directories(${PROJECT_NAME} PRIVATE .)
target_link_libraries(${PROJECT_NAME} PRIVATE drogon)