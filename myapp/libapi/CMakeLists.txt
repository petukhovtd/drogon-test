project(myapp_libapi)

set(SOURCE
    src/auth.cpp
    src/checkers.cc
    src/error.cc
)

add_library(${PROJECT_NAME} ${SOURCE})

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC jsoncpp cpp-base64)

add_subdirectory(test)
