cmake_minimum_required (VERSION 3.10)

# Name of the project
project (chess_gui)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Og -g -pg")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")

set(CMAKE_MODULE_PATH "${CMAKE_CXX_FLAGS}" "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(SDL2 REQUIRED)
include_directories(chess_gui PRIVATE ${SDL2_INCLUDE_DIRS})

add_subdirectory(ext)
include_directories(chess_gui PRIVATE ext/aelib)
include_directories(chess_gui PRIVATE ext/imgui)
include_directories(chess_gui PRIVATE ext/imgui/backends)

add_subdirectory(src)
install(TARGETS chess_gui DESTINATION bin)
