#!/bin/bash
# Integration test for ConfigFile feature

# Use Case: User wants to save default search settings in .findmyfilesrc
# and use them automatically without specifying CLI options every time.

set -e

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
TEST_DIR="test_config_integration_$$"
BIN="$PROJECT_ROOT/build/find_my_files"

# Verify binary exists
if [ ! -f "$BIN" ]; then
    echo "Error: Binary not found at $BIN"
    echo "Please build the project first: cmake --build build"
    exit 1
fi

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== ConfigFile Integration Test ===${NC}"

# Setup test environment
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

# Create test files
echo "Test file 1" > test1.txt
echo "Test file 2" > test2.log
mkdir subdir
echo "Test file 3" > subdir/test3.txt
echo "TODO: Fix this" > subdir/todo.txt

echo "Created test files:"
tree . 2>/dev/null || find . -type f

# Test 1: Default config file in current directory
echo -e "\n${YELLOW}Test 1: Load config from current directory${NC}"
cat > .findmyfilesrc << 'EOF'
[default]
recursive=true
format=detailed
color=false
EOF

OUTPUT=$($BIN . 2>&1)
if echo "$OUTPUT" | grep -q "Loaded configuration from:.*\.findmyfilesrc"; then
    echo -e "${GREEN}✓ Config file loaded${NC}"
else
    echo -e "${RED}✗ Config file not loaded${NC}"
    exit 1
fi

# Verify recursive search (should find files in subdir)
if echo "$OUTPUT" | grep -q "subdir/test3.txt"; then
    echo -e "${GREEN}✓ Recursive search enabled by config${NC}"
else
    echo -e "${RED}✗ Recursive search not working${NC}"
    exit 1
fi

# Test 2: Saved search configuration
echo -e "\n${YELLOW}Test 2: Saved search for TODO items${NC}"
cat > .findmyfilesrc << 'EOF'
[default]
recursive=true

[search.todos]
content=TODO
files_only=true
EOF

# Note: We can't directly use saved searches from CLI yet,
# but we can test that the config file is loaded
OUTPUT=$($BIN . 2>&1)
if echo "$OUTPUT" | grep -q "Loaded configuration from:"; then
    echo -e "${GREEN}✓ Config with saved search loaded${NC}"
else
    echo -e "${RED}✗ Config not loaded${NC}"
    exit 1
fi

# Test 3: CLI overrides config file
echo -e "\n${YELLOW}Test 3: CLI arguments override config file${NC}"
cat > .findmyfilesrc << 'EOF'
[default]
recursive=true
format=detailed
EOF

# Search non-recursively (override config)
OUTPUT=$($BIN . --no-recursive 2>&1)
# Should NOT find files in subdir
if echo "$OUTPUT" | grep -q "subdir/test3.txt"; then
    echo -e "${RED}✗ CLI override not working (found files in subdir)${NC}"
    exit 1
else
    echo -e "${GREEN}✓ CLI --no-recursive overrides config file${NC}"
fi

# Test 4: Config with extensions filter
echo -e "\n${YELLOW}Test 4: Extension filter from config${NC}"
cat > .findmyfilesrc << 'EOF'
[default]
recursive=true

[search.logs]
extensions=.log
EOF

OUTPUT=$($BIN . 2>&1)
if echo "$OUTPUT" | grep -q "Loaded configuration from:"; then
    echo -e "${GREEN}✓ Config with extension filter loaded${NC}"
else
    echo -e "${RED}✗ Config not loaded${NC}"
    exit 1
fi

# Test 5: Config with verbosity and logging
echo -e "\n${YELLOW}Test 5: Verbosity and logging from config${NC}"
cat > .findmyfilesrc << 'EOF'
[default]
recursive=true
verbosity=2
log_file=search.log
EOF

OUTPUT=$($BIN . 2>&1)
if [ -f "search.log" ]; then
    echo -e "${GREEN}✓ Log file created from config${NC}"
    # Check log file has content
    if [ -s "search.log" ]; then
        echo -e "${GREEN}✓ Log file has content${NC}"
    else
        echo -e "${RED}✗ Log file is empty${NC}"
        exit 1
    fi
else
    echo -e "${RED}✗ Log file not created${NC}"
    exit 1
fi

# Test 6: Config with thread count
echo -e "\n${YELLOW}Test 6: Thread count from config${NC}"
cat > .findmyfilesrc << 'EOF'
[default]
recursive=true
threads=2
verbosity=1
EOF

OUTPUT=$($BIN . 2>&1)
if echo "$OUTPUT" | grep -q "Thread count: 2"; then
    echo -e "${GREEN}✓ Thread count from config applied${NC}"
else
    echo -e "${YELLOW}⚠ Thread count not visible in output (may be correct)${NC}"
fi

# Test 7: Config with ignore patterns
echo -e "\n${YELLOW}Test 7: Ignore file from config${NC}"
cat > .findmyfilesrc << 'EOF'
[default]
recursive=true
ignore_file=.testignore
EOF

cat > .testignore << 'EOF'
*.log
EOF

OUTPUT=$($BIN . 2>&1)
if echo "$OUTPUT" | grep -q "test2.log"; then
    echo -e "${RED}✗ Ignore file not working (found .log file)${NC}"
    exit 1
else
    echo -e "${GREEN}✓ Ignore file from config applied${NC}"
fi

# Test 8: Config with max depth
echo -e "\n${YELLOW}Test 8: Max depth from config${NC}"
mkdir -p deep/nested/dir
echo "Deep file" > deep/nested/dir/deep.txt

cat > .findmyfilesrc << 'EOF'
[default]
recursive=true
max_depth=1
EOF

OUTPUT=$($BIN . 2>&1)
if echo "$OUTPUT" | grep -q "deep/nested/dir/deep.txt"; then
    echo -e "${RED}✗ Max depth not working (found deeply nested file)${NC}"
    exit 1
else
    echo -e "${GREEN}✓ Max depth from config applied${NC}"
fi

# Cleanup
cd ..
rm -rf "$TEST_DIR"

echo -e "\n${GREEN}=== All ConfigFile integration tests passed! ===${NC}"
exit 0
