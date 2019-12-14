cmake_minimum_required (VERSION 2.6)

project (Example)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

add_subdirectory(src)


add_executable(Example example/main.cpp)
add_executable(SpeedTest test/test.cpp)


target_link_libraries(Example PRIVATE TaskSystem)
target_link_libraries(SpeedTest PRIVATE TaskSystem)