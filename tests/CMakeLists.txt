# Test configuration
find_package(GTest CONFIG QUIET)
if (NOT GTest_FOUND)
    message(STATUS "GTest not found. Fetching GoogleTest...")
    set(GTEST_VERSION_TAG "v1.17.0")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_TAG ${GTEST_VERSION_TAG}
        GIT_REPOSITORY https://github.com/google/googletest.git
        )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()
message(STATUS "GoogleTest version: ${GTest_VERSION}")

# Create test executable
add_executable(solid_index_parser_tests
    solid_index_parser_tests.cpp
    ${CMAKE_SOURCE_DIR}/src/solid_index_parser.cpp
)

target_include_directories(solid_index_parser_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(solid_index_parser_tests
    GTest::gtest
    GTest::gtest_main
)

target_compile_options(solid_index_parser_tests PRIVATE ${MY_COMPILE_OPTIONS})
target_compile_definitions(solid_index_parser_tests PUBLIC ${MY_COMPILE_DEFINITIONS})
target_compile_features(solid_index_parser_tests PRIVATE ${MY_CXX_FEATURE})

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(solid_index_parser_tests
    DISCOVERY_MODE POST_BUILD
    DISCOVERY_TIMEOUT 30
)

