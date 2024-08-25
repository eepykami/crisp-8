cmake_minimum_required(VERSION 3.13)
project(crisp-8)

# Set include directories
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${SDL2_INCLUDE_DIR})

# SDL2
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)

# Windows stuff
if (WIN32) 
    add_compile_definitions(NOMINMAX)             # Make windows.h not define min/max macros because third-party deps don't like it
    add_compile_definitions(WIN32_LEAN_AND_MEAN)  # Make windows.h not include literally everything
    add_compile_definitions(SDL_MAIN_HANDLED)
endif()

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

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

if(MSVC)
    target_link_options( ${PROJECT_NAME} PUBLIC /subsystem:windows 
                                                /ENTRY:mainCRTStartup
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE SDL2-static)