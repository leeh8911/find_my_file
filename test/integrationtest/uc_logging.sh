#!/bin/bash
# Integration Test: Logging functionality
# Tests verbose modes and log file output

# Setup
TEST_DIR="${BASH_SOURCE%/*}"
PROJECT_ROOT="$TEST_DIR/../.."
BINARY="$PROJECT_ROOT/build/find_my_files"
TEST_LOGS_DIR="$TEST_DIR/test_logs"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Test counter
TESTS_RUN=0
TESTS_PASSED=0

# Test function
run_test() {
    local test_name="$1"
    TESTS_RUN=$((TESTS_RUN + 1))
    echo "Running: $test_name"
}

pass_test() {
    TESTS_PASSED=$((TESTS_PASSED + 1))
    echo -e "${GREEN}PASS${NC}: $1"
}

fail_test() {
    echo -e "${RED}FAIL${NC}: $1"
}

# Create test environment
rm -rf "$TEST_LOGS_DIR"
mkdir -p "$TEST_LOGS_DIR"

echo "========================================"
echo "Logging Functionality Integration Tests"
echo "========================================"

# Test 1: Default mode (no logging output)
run_test "Test 1: Default mode with no verbose flag"
OUTPUT=$("$BINARY" "$TEST_DIR" 2>&1)
if ! echo "$OUTPUT" | grep -q "\[INFO\]"; then
    pass_test "No log output in default mode"
else
    fail_test "Log output found in default mode"
fi

# Test 2: Info mode (-v)
run_test "Test 2: Info mode with -v flag"
OUTPUT=$("$BINARY" -v "$TEST_DIR" 2>&1)
if echo "$OUTPUT" | grep -q "\[INFO\]" && ! echo "$OUTPUT" | grep -q "\[DEBUG\]"; then
    pass_test "INFO level logging with -v"
else
    fail_test "INFO level not working correctly"
fi

# Test 3: Debug mode (-v -v)
run_test "Test 3: Debug mode with -v -v flags"
OUTPUT=$("$BINARY" -v -v "$TEST_DIR" 2>&1)
if echo "$OUTPUT" | grep -q "\[DEBUG\]" && echo "$OUTPUT" | grep -q "\[INFO\]"; then
    pass_test "DEBUG level logging with -v -v"
else
    fail_test "DEBUG level not working correctly"
fi

# Test 4: Log file creation
run_test "Test 4: Log file creation with --log-file"
LOG_FILE="$TEST_LOGS_DIR/test4.log"
"$BINARY" -v --log-file "$LOG_FILE" "$TEST_DIR" > /dev/null 2>&1
if [ -f "$LOG_FILE" ] && [ -s "$LOG_FILE" ]; then
    pass_test "Log file created and has content"
else
    fail_test "Log file not created or empty"
fi

# Test 5: Log file contains expected content
run_test "Test 5: Log file contains INFO and timestamps"
LOG_FILE="$TEST_LOGS_DIR/test5.log"
"$BINARY" -v --log-file "$LOG_FILE" "$TEST_DIR" > /dev/null 2>&1
if grep -q "\[INFO\]" "$LOG_FILE" && grep -q "[0-9]\{4\}-[0-9]\{2\}-[0-9]\{2\}" "$LOG_FILE"; then
    pass_test "Log file contains INFO level and timestamps"
else
    fail_test "Log file missing expected content"
fi

# Test 6: Debug log file contains more detail
run_test "Test 6: Debug log file with -v -v"
LOG_FILE="$TEST_LOGS_DIR/test6.log"
"$BINARY" -v -v --log-file "$LOG_FILE" "$TEST_DIR" > /dev/null 2>&1
if grep -q "\[DEBUG\]" "$LOG_FILE" && grep -q "\[INFO\]" "$LOG_FILE"; then
    pass_test "Debug log file contains both DEBUG and INFO"
else
    fail_test "Debug log file missing expected levels"
fi

# Test 7: Recursive scan debug logging
run_test "Test 7: Recursive scan with debug logging"
LOG_FILE="$TEST_LOGS_DIR/test7.log"
"$BINARY" -r -v -v --log-file "$LOG_FILE" "$TEST_DIR" > /dev/null 2>&1
if grep -q "Starting recursive scan" "$LOG_FILE" || grep -q "Recursive: yes" "$LOG_FILE"; then
    pass_test "Recursive scan debug messages present"
else
    fail_test "Recursive scan debug messages missing"
fi

# Test 8: Error handling without verbose (stderr only)
run_test "Test 8: Error on non-existent directory"
ERROR_OUTPUT=$("$BINARY" /nonexistent/path 2>&1)
if echo "$ERROR_OUTPUT" | grep -q -i "does not exist"; then
    pass_test "Error message displayed for non-existent path"
else
    fail_test "No error message for non-existent path"
fi

# Test 9: Combined options test
run_test "Test 9: Verbose with recursive and multiple options"
LOG_FILE="$TEST_LOGS_DIR/test9.log"
"$BINARY" -r -v --log-file "$LOG_FILE" -n "*.md" "$TEST_DIR" > /dev/null 2>&1
if [ -f "$LOG_FILE" ] && grep -q "\[INFO\]" "$LOG_FILE"; then
    pass_test "Combined options work with logging"
else
    fail_test "Combined options failed with logging"
fi

# Test 10: Log file in subdirectory
run_test "Test 10: Log file in nested subdirectory"
NESTED_LOG="$TEST_LOGS_DIR/nested/test10.log"
mkdir -p "$TEST_LOGS_DIR/nested"
"$BINARY" -v --log-file "$NESTED_LOG" "$TEST_DIR" > /dev/null 2>&1
if [ -f "$NESTED_LOG" ] && [ -s "$NESTED_LOG" ]; then
    pass_test "Log file created in nested directory"
else
    fail_test "Failed to create log in nested directory"
fi

# Cleanup
rm -rf "$TEST_LOGS_DIR"

# Summary
echo "========================================"
echo "Tests run: $TESTS_RUN"
echo "Tests passed: $TESTS_PASSED"
if [ $TESTS_RUN -eq $TESTS_PASSED ]; then
    echo -e "${GREEN}ALL TESTS PASSED${NC}"
    exit 0
else
    echo -e "${RED}SOME TESTS FAILED${NC}"
    exit 1
fi
