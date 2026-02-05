cmake_minimum_required(VERSION 3.16)

project(libcsv_modern
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(legacy)
add_subdirectory(csvcpp)
add_subdirectory(examples)
