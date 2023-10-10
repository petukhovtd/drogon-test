project(myapp_test)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE gtest drogon myapp_libapi)

add_test(myappTests ${PROJECT_NAME})
add_dependencies(build-test ${PROJECT_NAME})
