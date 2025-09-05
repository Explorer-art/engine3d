cmake_minimum_required(VERSION 2.8)

project(Engine3D)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
#include_directories(include)

add_executable(main src/main.c src/engine.c src/vector.c src/matrix.c)
target_link_libraries(main ${CURSES_LIBRARIES} m)
