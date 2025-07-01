cmake_minimum_required(VERSION 3.10)
project(EmulatorProject C)

# Enable debug symbols
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

# Find SDL2 and SDL2_ttf using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

# Include SDL2 and SDL2_ttf headers
include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

# Add the executable
add_executable(emulator.out
    main.c
    cpu.c
    utilities.c
    emulator.c
)

# Link SDL2 and SDL2_ttf
target_link_libraries(emulator.out
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
)
