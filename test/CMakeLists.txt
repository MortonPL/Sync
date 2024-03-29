enable_testing()
# dependencies
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# sources
set(TEST_FILES
    main.cpp
    Blocking.cpp
    Creeping.cpp
    Mapping.cpp
    Pairing.cpp
    Syncing.cpp
)

add_executable(${TEST_NAME} ${TEST_FILES})
target_include_directories(${TEST_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src/headers")
target_link_libraries(${TEST_NAME} GTest::gtest_main ${LIB_NAME})
gtest_discover_tests(${TEST_NAME})

install(TARGETS ${TEST_NAME} DESTINATION bin COMPONENT ${TEST_NAME})
