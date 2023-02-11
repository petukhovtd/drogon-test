project(myapp)

add_subdirectory(user)

set(SOURCE
    error.cc
    checkers.cc
    api_v1_User.cc
)

add_executable(${PROJECT_NAME} myapp.cpp ${SOURCE})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE myapp_user drogon cpp-base64)
