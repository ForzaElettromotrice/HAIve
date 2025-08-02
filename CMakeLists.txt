cmake_minimum_required(VERSION 3.29)
project(HAIve CXX C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_C_FLAGS "-O3")

add_compile_options(-Wall)
add_compile_definitions(Debug)

add_definitions(-DMODEL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/checkpoint.jit")
add_definitions(-DTEST_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test.txt")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/CUtils)
set(Torch_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libtorch/share/cmake/Torch)
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

find_package(Threads REQUIRED)

add_executable(HAIve
        src/main.cpp
        src/utils.c
        src/enums.c
        src/moves.c
        src/trainings.cpp
        src/hivecnn.cpp
        src/preprocessor.cpp
        src/heuristics.cpp
        src/tree.cpp
        src/minmanager.cpp
        src/HQueue.c
)

target_include_directories(
        HAIve PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(HAIve PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
        CUDA_SEPARABLE_COMPILATION ON
)

target_link_libraries(
        HAIve PRIVATE Threads::Threads CUtils ${TORCH_LIBRARIES}
)

#if (MSVC)
#    file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
#    add_custom_command(TARGET HAIve
#            POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            ${TORCH_DLLS}
#            $<TARGET_FILE_DIR:HAIve>)
#endif (MSVC)
