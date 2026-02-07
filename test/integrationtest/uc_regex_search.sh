#!/bin/bash
# Integration test: Regex search functionality

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_DATA_DIR="$SCRIPT_DIR/../testdata"
BUILD_DIR="$SCRIPT_DIR/../../build"
FMF_BIN="$BUILD_DIR/find_my_files"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo "=== UC: Regex Search Test ==="

# Setup test data
mkdir -p "$TEST_DATA_DIR/regex_test"
echo "test content" > "$TEST_DATA_DIR/regex_test/test_file_123.txt"
echo "test content" > "$TEST_DATA_DIR/regex_test/file_abc.txt"
echo "test content" > "$TEST_DATA_DIR/regex_test/document.txt"

# Test 1: Regex pattern matching
echo "Test 1: Match files with digits"
RESULT=$("$FMF_BIN" "$TEST_DATA_DIR/regex_test" -n ".*[0-9]+.*" --regex | grep -c "test_file_123.txt" || true)
if [ "$RESULT" -eq 1 ]; then
    echo -e "${GREEN}✓ Test 1 passed${NC}"
else
    echo -e "${RED}✗ Test 1 failed${NC}"
    exit 1
fi

# Test 2: Regex pattern not matching
echo "Test 2: Exclude files without digits"
RESULT=$("$FMF_BIN" "$TEST_DATA_DIR/regex_test" -n ".*[0-9]+.*" --regex | grep -c "document.txt" || true)
if [ "$RESULT" -eq 0 ]; then
    echo -e "${GREEN}✓ Test 2 passed${NC}"
else
    echo -e "${RED}✗ Test 2 failed${NC}"
    exit 1
fi

# Test 3: Regex with alternation
echo "Test 3: Match with alternation pattern"
RESULT=$("$FMF_BIN" "$TEST_DATA_DIR/regex_test" -n "(test|file)_.*" --regex | wc -l)
if [ "$RESULT" -ge 2 ]; then
    echo -e "${GREEN}✓ Test 3 passed${NC}"
else
    echo -e "${RED}✗ Test 3 failed${NC}"
    exit 1
fi

# Cleanup
rm -rf "$TEST_DATA_DIR/regex_test"

echo -e "${GREEN}=== All regex search tests passed ===${NC}"
