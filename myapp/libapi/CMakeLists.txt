project(myapp_libapi)

set(SOURCE
    src/checkers.cc
    src/error.cc
)

add_library(${PROJECT_NAME} ${SOURCE})

target_include_directories(${PROJECT_NAME} PUBLIC include)
