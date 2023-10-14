project(myapp_api_test)

set(SOURCE
    test_helpers.cpp
    test_user_create.cpp
)

add_executable(${PROJECT_NAME} test_main.cpp ${SOURCE})
target_link_libraries(${PROJECT_NAME} PRIVATE gtest drogon myapp_api myapp_libapi)

add_test(myappTests ${PROJECT_NAME})
add_dependencies(build-test ${PROJECT_NAME})
