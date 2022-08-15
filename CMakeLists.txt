cmake_minimum_required (VERSION 3.10)

# Name of the project
project (chess)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Og -g -pg")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")

set(CMAKE_MODULE_PATH "${CMAKE_CXX_FLAGS}" "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(ext/fmt EXCLUDE_FROM_ALL)
include_directories(chess PRIVATE ext/fmt/include)

add_subdirectory(ext/aelib)
include_directories(chess PRIVATE ext/aelib)

add_subdirectory(src)
add_subdirectory(docs)

add_custom_target(chess_meta DEPENDS chess run_chess test_chess chess_game)

install(TARGETS chess DESTINATION bin)
