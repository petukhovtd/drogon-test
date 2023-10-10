project(myapp_libapi_test)

set(SRC
        test_checkers.cpp
        test_auth.cpp
)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE myapp_libapi gtest gtest_main)

add_test(libapiTest ${PROJECT_NAME})
add_dependencies(build-test ${PROJECT_NAME})
