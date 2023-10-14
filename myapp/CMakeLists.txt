project(myapp)

add_subdirectory(user)
add_subdirectory(libapi)
add_subdirectory(api)

set(SOURCE
        program_args.cpp
)

add_executable(${PROJECT_NAME} main.cpp ${SOURCE})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE myapp_api cxxopts)
