enable_testing()
# dependencies
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# sources
set(TEST_FILES
    example_test.cpp
)

add_executable(${TEST_NAME} ${TEST_FILES})
target_link_libraries(${TEST_NAME} GTest::gtest_main ${LIB_NAME})
#gtest_discover_tests(${TEST_NAME})
add_test(NAME example_test COMMAND example)
