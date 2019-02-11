set(MYRMO_TESTS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(MYRMO_TESTS_EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)

include_directories(${MYRMO_INCLUDE_DIR})
include_directories(${MYRMO_TESTS_INCLUDE_DIR})
include_directories(${MYRMO_TESTS_EXTERNAL_DIR})

add_subdirectory(hash)
add_subdirectory(cache)

