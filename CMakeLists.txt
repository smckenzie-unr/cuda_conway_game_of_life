cmake_minimum_required(VERSION 3.22.1)

project(
    nvidia_conways_gol
    LANGUAGES C CXX CUDA
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_ARCHITECTURES 86)

find_package(CUDAToolkit REQUIRED)
find_package(SDL2 REQUIRED)

add_executable(
    nvidia_conways_gol
    sources/main.cpp
    sources/arg_parser.cpp
    sources/mainwindow.cpp
    sources/cuda_cntrl.cpp
    sources/conway_gol_kernel.cu
)

set_source_files_properties(
    sources/cuda_cntrl.cpp
    PROPERTIES
        LANGUAGE CUDA
)

set_target_properties(
    nvidia_conways_gol
    PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
)

target_include_directories(
    nvidia_conways_gol
    PRIVATE
        includes
)

target_link_libraries(
    nvidia_conways_gol
    PRIVATE
        CUDA::cudart
        SDL2::SDL2
)

target_compile_definitions(
    nvidia_conways_gol
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
        BLOCK_WIDTH_DIMENSION=16
        BLOCK_HEIGHT_DIMENSION=16
)