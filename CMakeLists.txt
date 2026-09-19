cmake_minimum_required(VERSION 4.3)

project(chip-8-emulator)

include_directories(include)

add_executable(chip_eight)

target_sources(chip_eight
  PRIVATE
    main.cpp
    src/Chip8.cpp
)