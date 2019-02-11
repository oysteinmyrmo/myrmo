cmake_minimum_required(VERSION 3.1)
project(Myrmo)

include(cmake/CMakeRC.cmake)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(MYRMO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(MYRMO_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

enable_testing()
add_subdirectory(${MYRMO_TESTS_DIR})
