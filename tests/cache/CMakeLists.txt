cmrc_add_resource_library(cache_test_data
	NAMESPACE test_data
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/103.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/106.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/107.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/108.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/109.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/110.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/111.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/112.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/113.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/114.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/115.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/116.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/117.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/118.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/119.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/120.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/121.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/122.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/123.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/124.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/125.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/126.JPG
	${CMAKE_CURRENT_SOURCE_DIR}/test_data/128.JPG
)

set(MYRMO_TESTS_CACHE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cache_dir)

add_executable(disk-cache-tests disk-cache-tests.cpp ${MYRMO_INCLUDE_DIR})
target_link_libraries(disk-cache-tests PRIVATE cache_test_data)
target_compile_definitions(disk-cache-tests PRIVATE -DMYRMO_TESTS_CACHE_DIR="${MYRMO_TESTS_CACHE_DIR}")
add_test(NAME disk-cache-tests COMMAND disk-cache-test)

add_executable(memory-cache-tests memory-cache-tests.cpp ${MYRMO_INCLUDE_DIR})
target_link_libraries(memory-cache-tests PRIVATE cache_test_data)
add_test(NAME memory-cache-tests COMMAND memory-cache-test)

