cmake_minimum_required(VERSION 3.16)
project(chess_engine_oop CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_library(engine_core
    src/board.cpp
    src/piece.cpp
    src/movegen.cpp
    src/eval.cpp
    src/search.cpp
)
target_include_directories(engine_core PUBLIC include)

add_executable(perft_test tests/perft_test.cpp)
target_link_libraries(perft_test PRIVATE engine_core)

add_executable(search_demo tests/search_demo.cpp)
target_link_libraries(search_demo PRIVATE engine_core)