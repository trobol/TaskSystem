cmake_minimum_required (VERSION 2.6)

project (Example)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes")

add_subdirectory(src)


add_executable(Example example/main.cpp)
