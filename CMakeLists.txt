cmake_minimum_required(VERSION 3.10)
project(crisp-8)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(crisp-8 main.c)
target_link_libraries(crisp-8 ${SDL2_LIBRARIES})