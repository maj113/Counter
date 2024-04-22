cmake_minimum_required(VERSION 3.5)

project(Counter)

set(CMAKE_CXX_STANDARD 17)
include_directories(${CMAKE_SOURCE_DIR})

if(MSVC)
    add_compile_options(/GA /EHsc /fp:fast /MP /arch:AVX2)
    
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        add_compile_options(/Od /Zi /Zf)
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "Release")
        add_compile_options(/O2)
    endif()
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -march=native -fdiagnostics-color=auto)
    
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        add_compile_options(-Og -g3)
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "Release")
        add_compile_options(-Ofast -funroll-loops -fno-exceptions)
    endif()
endif()

add_subdirectory(benchmark)
add_subdirectory(verify)
