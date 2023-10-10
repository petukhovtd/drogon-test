project(myapp)

add_subdirectory(user)
add_subdirectory(libapi)

set(SOURCE
        api/v1/user.cc
        program_args.cpp
)

add_executable(${PROJECT_NAME} myapp.cpp ${SOURCE})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE myapp_user myapp_libapi drogon cxxopts)

add_subdirectory(tests)
