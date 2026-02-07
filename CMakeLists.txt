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

# Build options
option(ENABLE_LOCAL_EMBEDDING "Enable local embedding with ONNX Runtime" OFF)

# Find ONNX Runtime (optional)
if(ENABLE_LOCAL_EMBEDDING)
    find_package(onnxruntime QUIET)
    if(onnxruntime_FOUND)
        message(STATUS "ONNX Runtime found - Local embedding enabled")
        add_compile_definitions(ENABLE_ONNX_RUNTIME)
    else()
        message(WARNING "ONNX Runtime not found - Local embedding will use placeholder implementation")
    endif()
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES

    # Domain Layer
    src/domain/entities/file_info.cpp
    src/domain/entities/search_result.cpp
    src/domain/value_objects/search_criteria.cpp

    # Application Layer
    src/application/services/pattern_matcher.cpp
    src/application/services/content_searcher.cpp
    src/application/services/ignore_patterns.cpp
    src/application/use_cases/file_scanner.cpp
    src/application/use_cases/semantic_searcher.cpp

    # Adapters Layer
    src/adapters/cli/command_line_parser.cpp
    src/adapters/presenters/output_formatter.cpp

    # Infrastructure Layer
    src/infrastructure/logging/logger.cpp
    src/infrastructure/config/config_file.cpp
    src/infrastructure/threading/thread_pool.cpp
    src/infrastructure/ai/local_embedding_provider.cpp
)

# Header files
set(HEADERS

    # Domain Layer
    include/domain/entities/file_info.h
    include/domain/entities/search_result.h
    include/domain/entities/semantic_result.h
    include/domain/value_objects/search_criteria.h

    # Application Layer
    include/application/ports/i_embedding_provider.h
    include/application/ports/i_vector_store.h
    include/application/services/pattern_matcher.h
    include/application/services/content_searcher.h
    include/application/services/ignore_patterns.h
    include/application/use_cases/file_scanner.h
    include/application/use_cases/semantic_searcher.h

    # Adapters Layer
    include/adapters/cli/command_line_parser.h
    include/adapters/presenters/output_formatter.h

    # Infrastructure Layer
    include/infrastructure/logging/logger.h
    include/infrastructure/config/config_file.h
    include/infrastructure/threading/thread_pool.h
    include/infrastructure/ai/local_embedding_provider.h
    include/infrastructure/ai/simple_tokenizer.h
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

    # Domain Layer Tests
    tests/domain/entities/test_file_info.cpp
    tests/domain/value_objects/test_search_criteria.cpp

    # Application Layer Tests
    tests/application/services/test_pattern_matcher.cpp
    tests/application/services/test_content_searcher.cpp
    tests/application/services/test_ignore_patterns.cpp
    tests/application/use_cases/test_file_scanner.cpp
    tests/application/use_cases/test_semantic_interfaces.cpp
    tests/application/use_cases/test_semantic_searcher.cpp

    # Adapters Layer Tests
    tests/adapters/test_output_formatter.cpp

    # Infrastructure Layer Tests
    tests/infrastructure/test_logger.cpp
    tests/infrastructure/test_config_file.cpp
    tests/infrastructure/test_thread_pool.cpp
    tests/infrastructure/test_local_embedding_provider.cpp
    tests/infrastructure/test_simple_tokenizer.cpp

    ${SOURCES}
)

target_link_libraries(tests
    GTest::gtest_main
    pthread
)

include(GoogleTest)
gtest_discover_tests(tests)

# Install target
install(TARGETS find_my_files DESTINATION bin)
