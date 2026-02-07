#!/bin/bash
# Integration test: Content search functionality

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_DATA_DIR="$SCRIPT_DIR/../testdata"
BUILD_DIR="$SCRIPT_DIR/../../build"
FMF_BIN="$BUILD_DIR/find_my_files"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo "=== UC: Content Search Test ==="

# Setup test data
mkdir -p "$TEST_DATA_DIR/content_test"
echo "This file contains the secret keyword" > "$TEST_DATA_DIR/content_test/file1.txt"
echo "This file has different content" > "$TEST_DATA_DIR/content_test/file2.txt"
echo "Another file with secret data" > "$TEST_DATA_DIR/content_test/file3.txt"
printf '\x00\x01\x02\x03' > "$TEST_DATA_DIR/content_test/binary.bin"

# Test 1: Find files with specific content
echo "Test 1: Search for 'secret' keyword"
RESULT=$("$FMF_BIN" "$TEST_DATA_DIR/content_test" -c "secret" | grep -c "\.txt" || true)
if [ "$RESULT" -eq 2 ]; then
    echo -e "${GREEN}✓ Test 1 passed${NC}"
else
    echo -e "${RED}✗ Test 1 failed (expected 2, got $RESULT)${NC}"
    exit 1
fi

# Test 2: Content pattern with regex
echo "Test 2: Search with regex pattern"
RESULT=$("$FMF_BIN" "$TEST_DATA_DIR/content_test" -c "file.*different" -x | grep -c "file2.txt" || true)
if [ "$RESULT" -eq 1 ]; then
    echo -e "${GREEN}✓ Test 2 passed${NC}"
else
    echo -e "${RED}✗ Test 2 failed${NC}"
    exit 1
fi

# Test 3: Binary file exclusion
echo "Test 3: Exclude binary files from content search"
RESULT=$("$FMF_BIN" "$TEST_DATA_DIR/content_test" -c "." | grep -c "binary.bin" || true)
if [ "$RESULT" -eq 0 ]; then
    echo -e "${GREEN}✓ Test 3 passed${NC}"
else
    echo -e "${RED}✗ Test 3 failed (binary file should be excluded)${NC}"
    exit 1
fi

# Test 4: Case insensitive content search
echo "Test 4: Case insensitive search"
RESULT=$("$FMF_BIN" "$TEST_DATA_DIR/content_test" -c "SECRET" -i | grep -c "\.txt" || true)
if [ "$RESULT" -eq 2 ]; then
    echo -e "${GREEN}✓ Test 4 passed${NC}"
else
    echo -e "${RED}✗ Test 4 failed${NC}"
    exit 1
fi

# Cleanup
rm -rf "$TEST_DATA_DIR/content_test"

echo -e "${GREEN}=== All content search tests passed ===${NC}"
