project(tests)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -std=c++11 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -std=c++11 -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

include_directories("../src/")
include_directories(".")

file(GLOB all_tests_src
    "src/*.cpp"
    "../src/chip8.cpp"
)

add_executable(tests ${all_tests_src})

if(APPLE)
    target_link_libraries(tests)
elseif(UNIX)
    target_link_libraries(tests pthread)
endif()
