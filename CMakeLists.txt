cmake_minimum_required(VERSION 3.16)

project(GeoSearch VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(CPP_HTTPLIB REQUIRED IMPORTED_TARGET cpp-httplib)

# Main data structure demonstration application
add_executable(geosearch
    src/main.cpp
)

# HTTP server
add_executable(geosearch_server
    src/server.cpp
)

# Benchmark application
add_executable(index_benchmark
    benchmarks/index_benchmark.cpp
)

# Correctness test application
add_executable(index_tests
    tests/index_tests.cpp
)

# Include directories
foreach(target
    geosearch
    geosearch_server
    index_benchmark
    index_tests
)
    target_include_directories(${target} PRIVATE
        ${PROJECT_SOURCE_DIR}/include
    )

    target_compile_options(${target} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endforeach()

# HTTP server dependencies
target_link_libraries(geosearch_server PRIVATE
    PkgConfig::CPP_HTTPLIB
    Threads::Threads
)

# Register correctness tests
enable_testing()

add_test(
    NAME GeoSearchIndexTests
    COMMAND index_tests
)