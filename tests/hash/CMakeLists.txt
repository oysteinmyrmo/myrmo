add_executable(sha1-tests sha1-tests.cpp long_strings.h ${MYRMO_INCLUDE_DIR})
add_test(NAME sha1-tests COMMAND sha1-tests)

