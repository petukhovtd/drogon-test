project(myapp_user)

set(SOURCE
    src/misc.cc
    src/user.cc
    src/user_db.cc
)

add_library(${PROJECT_NAME} ${SOURCE})

target_include_directories(${PROJECT_NAME} PUBLIC include)
