#!/bin/bash
# Run all integration tests

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Running All Integration Tests${NC}"
echo -e "${BLUE}========================================${NC}"
echo

# Check if binary exists
BUILD_DIR="$SCRIPT_DIR/../../build"
if [ ! -f "$BUILD_DIR/find_my_files" ]; then
    echo -e "${RED}Error: find_my_files binary not found in $BUILD_DIR${NC}"
    echo "Please build the project first: cd build && cmake .. && make"
    exit 1
fi

# Make all test scripts executable
chmod +x "$SCRIPT_DIR"/*.sh

# Run each test
TESTS=(
    "uc_regex_search.sh"
    "uc_content_search.sh"
    "uc_ignore_patterns.sh"
    "uc_parallel_scan.sh"
    "uc_output_formats.sh"
    "uc_semantic_search.sh"
)

PASSED=0
FAILED=0

for TEST in "${TESTS[@]}"; do
    echo -e "${BLUE}Running $TEST...${NC}"
    if bash "$SCRIPT_DIR/$TEST" 2>&1; then
        ((PASSED++))
        echo
    else
        ((FAILED++))
        echo -e "${RED}$TEST failed!${NC}"
        echo
    fi
done

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Test Results${NC}"
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}Passed: $PASSED${NC}"
if [ $FAILED -gt 0 ]; then
    echo -e "${RED}Failed: $FAILED${NC}"
    exit 1
else
    echo -e "${GREEN}All integration tests passed!${NC}"
fi
