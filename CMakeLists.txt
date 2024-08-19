cmake_minimum_required(VERSION 3.10)
project(crisp-8)

# Set include directories
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${SDL2_INCLUDE_DIR})

# SDL2
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)

# Add subdirectories
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/SDL)

# Set source files
set(SOURCES
    src/main.c
)

set(HEADERS
    include/font.h
    include/ibm_rom.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2-static)