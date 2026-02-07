cmake_minimum_required(VERSION 3.14)
project(find_my_files VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/file_info.cpp
    src/search_result.cpp
    src/file_scanner.cpp
    src/search_criteria.cpp
    src/pattern_matcher.cpp
    src/content_searcher.cpp
    src/ignore_patterns.cpp
)

# Header files
set(HEADERS
    include/file_info.h
    include/search_result.h
    include/file_scanner.h
    include/search_criteria.h
    include/pattern_matcher.h
    include/content_searcher.h
    include/ignore_patterns.h
)

# Main executable
add_executable(find_my_files
    src/main.cpp
    ${SOURCES}
)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Test executable
add_executable(tests
    tests/test_file_info.cpp
    tests/test_file_scanner.cpp
    tests/test_pattern_matcher.cpp
    tests/test_search_criteria.cpp
    tests/test_content_searcher.cpp
    tests/test_ignore_patterns.cpp
    ${SOURCES}
)

target_link_libraries(tests
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)

# Install target
install(TARGETS find_my_files DESTINATION bin)
