project(myapp_api)

add_subdirectory(test)

set(SOURCE
    src/api_user.cpp
    src/user_http_controller.cpp
)

add_library(${PROJECT_NAME} ${SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC myapp_user myapp_libapi drogon)
