cmake_minimum_required(VERSION 3.16)
project(chess_engine_oop CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

find_package(Threads REQUIRED)

# Core engine: board representation, move generation (Piece hierarchy),
# evaluation, single-threaded search, and the Lazy SMP pieces (transposition
# table, Zobrist hashing, parallel search). Everything links against this.
add_library(engine_core
    src/board.cpp
    src/piece.cpp
    src/movegen.cpp
    src/eval.cpp
    src/search.cpp
    src/tt.cpp
    src/zobrist.cpp
    src/parallel.cpp
    src/nnue_eval.cpp
)
target_include_directories(engine_core PUBLIC include)
target_link_libraries(engine_core PUBLIC Threads::Threads)

# The actual UCI-speaking engine binary — load this into a GUI.
add_executable(chess_engine
    src/main.cpp
    src/uci.cpp
)
target_link_libraries(chess_engine PRIVATE engine_core)

# perft: move generation correctness check against known node counts.
add_executable(perft_test tests/perft_test.cpp)
target_link_libraries(perft_test PRIVATE engine_core)

# perft_extended: startpos plus the standard castling/en-passant/promotion
# perft suite (Kiwipete, position 3, position 4) — exercises exactly the
# move types a make/unmake bug tends to hide in.
add_executable(perft_extended_test tests/perft_extended_test.cpp)
target_link_libraries(perft_extended_test PRIVATE engine_core)

# search_demo: sanity check for the single-threaded search() path.
add_executable(search_demo tests/search_demo.cpp)
target_link_libraries(search_demo PRIVATE engine_core)

# parallel_demo: compares node throughput at 1 / 2 / 4 threads under the
# same time budget — the actual evidence Lazy SMP is doing concurrent work.
add_executable(parallel_demo tests/parallel_demo.cpp)
target_link_libraries(parallel_demo PRIVATE engine_core)

add_executable(play_demo tests/play_demo.cpp)
target_link_libraries(play_demo PRIVATE engine_core)

add_executable(mate_test tests/mate_test.cpp)
target_link_libraries(mate_test PRIVATE engine_core)

add_executable(generate_selfplay_data tools/generate_selfplay_data.cpp)
target_link_libraries(generate_selfplay_data PRIVATE engine_core)

add_executable(nnue_demo tools/nnue_demo.cpp)
target_link_libraries(nnue_demo PRIVATE engine_core)

enable_testing()
add_test(NAME perft COMMAND perft_test)
add_test(NAME perft_extended COMMAND perft_extended_test)