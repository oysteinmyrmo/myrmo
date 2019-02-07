set(MYRMO_TESTS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(${MYRMO_INCLUDE_DIR})
include_directories(${MYRMO_TESTS_INCLUDE_DIR})

add_subdirectory(hash)

