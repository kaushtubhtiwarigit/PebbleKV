cmake_minimum_required(VERSION 3.14)
project(PebbleKV VERSION 0.1.0 LANGUAGES CXX)

# Require C++17 (C++20 preferred but C++17 works too)
# Change to 20 if you have a modern compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Main executable
add_executable(pebblekv
    src/main.cpp
    src/database.cpp
    src/command_parser.cpp
)

# Testing with GoogleTest
option(BUILD_TESTING "Build tests" ON)

if(BUILD_TESTING)
    # Fetch GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    enable_testing()
    
    add_executable(pebblekv_tests
        tests/test_database.cpp
        src/database.cpp
        src/command_parser.cpp
    )
    
    target_link_libraries(pebblekv_tests
        GTest::gtest_main
    )
    
    include(GoogleTest)
    gtest_discover_tests(pebblekv_tests)
endif()
