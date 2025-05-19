cmake_minimum_required(VERSION 3.29)
project(HAIve CXX C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_C_FLAGS "-O3")

#set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/libtorch)

#add_compile_options(-Wall /wd5045)
#add_compile_definitions(Debug _CRT_SECURE_NO_WARNINGS)
add_compile_definitions(Debug)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/CUtils)


set(CMAKE_PREFIX_PATH /opt/libtorch-cuda)
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")


add_executable(HAIve
        src/main.c
        src/moves.c
        src/logger.c
        src/utils.c
        src/hashmap.c
        src/enums.c
        src/logic.c
        src/preprocessor.cpp
)

target_include_directories(
        HAIve PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(HAIve PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
        CUDA_SEPARABLE_COMPILATION ON
)

target_link_libraries(
        HAIve PRIVATE CUtils ${TORCH_LIBRARIES}
)

#if (MSVC)
#    file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
#    add_custom_command(TARGET HAIve
#            POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            ${TORCH_DLLS}
#            $<TARGET_FILE_DIR:HAIve>)
#endif (MSVC)