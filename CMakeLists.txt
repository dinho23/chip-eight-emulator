cmake_minimum_required(VERSION 4.3)

project(chip-8-emulator)

find_package(SDL3 REQUIRED CONFIG)

add_executable(chip_eight)

target_compile_features(chip_eight PRIVATE cxx_std_17)

target_compile_definitions(chip_eight PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
)

target_sources(chip_eight
  PRIVATE
    src/main.cpp
    src/Chip8.cpp
)

target_include_directories(chip_eight PRIVATE include)

target_link_libraries(chip_eight PRIVATE SDL3::SDL3)