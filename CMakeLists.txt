cmake_minimum_required(VERSION 3.16)
project(pos LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(POS_WARNINGS /W4 /utf-8)
else()
    set(POS_WARNINGS -Wall -Wextra -Wpedantic)
endif()

add_executable(pos src/main.cpp src/application/facade.cpp)
target_include_directories(pos PRIVATE src)
target_compile_options(pos PRIVATE ${POS_WARNINGS})

enable_testing()

find_package(GTest QUIET)
if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(pos_tests tests/tests.cpp src/application/facade.cpp)
target_include_directories(pos_tests PRIVATE src)
target_compile_options(pos_tests PRIVATE ${POS_WARNINGS})
target_link_libraries(pos_tests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(pos_tests)