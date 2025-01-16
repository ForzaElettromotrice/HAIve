cmake_minimum_required(VERSION 3.29)
project(HAIve C)

set(CMAKE_C_STANDARD 17)
#set(CMAKE_C_FLAGS "-O3")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

add_compile_options(-Wall)

add_compile_definitions(Debug)

add_executable(HAIve src/main.c
        src/enums.h
        src/logger.c
        src/logger.h
        src/main.h
        src/engine.c
        src/engine.h)
