#!/bin/bash
# Integration test: Ignore patterns functionality

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_DATA_DIR="$SCRIPT_DIR/../testdata"
BUILD_DIR="$SCRIPT_DIR/../../build"
FMF_BIN="$BUILD_DIR/find_my_files"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo "=== UC: Ignore Patterns Test ==="

# Setup test data
mkdir -p "$TEST_DATA_DIR/ignore_test/node_modules"
mkdir -p "$TEST_DATA_DIR/ignore_test/build"
mkdir -p "$TEST_DATA_DIR/ignore_test/src"

echo "source file" > "$TEST_DATA_DIR/ignore_test/src/main.cpp"
echo "config" > "$TEST_DATA_DIR/ignore_test/package.json"
echo "dependency" > "$TEST_DATA_DIR/ignore_test/node_modules/lib.js"
echo "binary" > "$TEST_DATA_DIR/ignore_test/build/app"
echo "temp" > "$TEST_DATA_DIR/ignore_test/temp.tmp"

# Create .gitignore
cat > "$TEST_DATA_DIR/ignore_test/.gitignore" << EOF
node_modules/
build/
*.tmp
EOF

# Test 1: Ignore node_modules directory
echo "Test 1: Ignore node_modules directory"
RESULT=$("$FMF_BIN" "$TEST_DATA_DIR/ignore_test" -r --ignore "$TEST_DATA_DIR/ignore_test/.gitignore" | grep -c "node_modules" || true)
if [ "$RESULT" -eq 0 ]; then
    echo -e "${GREEN}✓ Test 1 passed${NC}"
else
    echo -e "${RED}✗ Test 1 failed (node_modules should be ignored)${NC}"
    exit 1
fi

# Test 2: Ignore build directory
echo "Test 2: Ignore build directory"
RESULT=$("$FMF_BIN" "$TEST_DATA_DIR/ignore_test" -r --ignore "$TEST_DATA_DIR/ignore_test/.gitignore" | grep -c "build/" || true)
if [ "$RESULT" -eq 0 ]; then
    echo -e "${GREEN}✓ Test 2 passed${NC}"
else
    echo -e "${RED}✗ Test 2 failed (build/ should be ignored)${NC}"
    exit 1
fi

# Test 3: Ignore *.tmp files
echo "Test 3: Ignore *.tmp files"
RESULT=$("$FMF_BIN" "$TEST_DATA_DIR/ignore_test" -r --ignore "$TEST_DATA_DIR/ignore_test/.gitignore" | grep -c "temp.tmp" || true)
if [ "$RESULT" -eq 0 ]; then
    echo -e "${GREEN}✓ Test 3 passed${NC}"
else
    echo -e "${RED}✗ Test 3 failed (*.tmp should be ignored)${NC}"
    exit 1
fi

# Test 4: Allow src directory
echo "Test 4: Allow src directory (not ignored)"
RESULT=$("$FMF_BIN" "$TEST_DATA_DIR/ignore_test" -r --ignore "$TEST_DATA_DIR/ignore_test/.gitignore" | grep -c "main.cpp" || true)
if [ "$RESULT" -eq 1 ]; then
    echo -e "${GREEN}✓ Test 4 passed${NC}"
else
    echo -e "${RED}✗ Test 4 failed (src/main.cpp should be included)${NC}"
    exit 1
fi

# Cleanup
rm -rf "$TEST_DATA_DIR/ignore_test"

echo -e "${GREEN}=== All ignore patterns tests passed ===${NC}"
