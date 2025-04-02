cmake_minimum_required(VERSION 3.29)
project(HAIve C)

set(CMAKE_C_STANDARD 17)
#set(CMAKE_C_FLAGS "-O3")

add_compile_options(-Wall)
add_compile_definitions(Debug)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/xxHash/cmake_unofficial ${CMAKE_CURRENT_SOURCE_DIR}/lib/xxHash EXCLUDE_FROM_ALL)

add_executable(HAIve
        src/main.c
        src/moves.c
        src/logger.c
        src/utils.c
)

target_include_directories(
        HAIve PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(HAIve PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)

target_link_libraries(
        HAIve PRIVATE xxHash::xxhash
)