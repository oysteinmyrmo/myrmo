set(MYRMO_TESTS_CACHE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cache_dir)

add_executable(disk-cache-tests disk-cache-tests.cpp ${MYRMO_INCLUDE_DIR})
add_test(NAME disk-cache-tests COMMAND disk-cache-tests)

target_compile_definitions(disk-cache-tests PRIVATE -DMYRMO_TESTS_CACHE_DIR="${MYRMO_TESTS_CACHE_DIR}")

