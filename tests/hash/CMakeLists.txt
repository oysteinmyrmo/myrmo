cmrc_add_resource_library(sha_test_data
    NAMESPACE test_data
    ${CMAKE_CURRENT_SOURCE_DIR}/test_data/lorem_ipsum_10_paragraphs.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/test_data/lorem_ipsum_10_paragraphs_raw.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/test_data/random_org_1000_20.txt)

add_executable(sha1-tests sha1-tests.cpp ${MYRMO_INCLUDE_DIR})
target_link_libraries(sha1-tests PRIVATE sha_test_data)
add_test(NAME sha1-tests COMMAND sha1-tests)

add_executable(crc-tests crc-tests.cpp ${MYRMO_INCLUDE_DIR})
target_link_libraries(crc-tests PRIVATE sha_test_data)
add_test(NAME crc-tests COMMAND crc-tests)
