#!/bin/bash
# Integration test for output format options

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/../../build"
FMF="$BUILD_DIR/find_my_files"
TEST_DIR="$SCRIPT_DIR/test_data"

# Create test directory structure
mkdir -p "$TEST_DIR"
echo "test content" > "$TEST_DIR/file1.txt"
echo "another test" > "$TEST_DIR/file2.txt"
mkdir -p "$TEST_DIR/subdir"
echo "nested file" > "$TEST_DIR/subdir/file3.txt"

echo "=== UC: Output Format Options ==="

# Test 1: Default format
echo "[Test 1] Default format output"
OUTPUT=$("$FMF" -r "$TEST_DIR" 2>&1)
if echo "$OUTPUT" | grep -q "Found 4 items"; then
    echo "✓ Default format shows item count"
else
    echo "✗ Default format missing item count"
    echo "Output was: $OUTPUT"
    exit 1
fi

if echo "$OUTPUT" | grep -q "\[FILE\].*file1.txt"; then
    echo "✓ Default format shows file type and name"
else
    echo "✗ Default format missing file info"
    exit 1
fi

# Test 2: Detailed format
echo "[Test 2] Detailed format output"
OUTPUT=$("$FMF" -r --format detailed "$TEST_DIR" 2>&1)
if echo "$OUTPUT" | grep -q "Path:.*file1.txt"; then
    echo "✓ Detailed format shows path label"
else
    echo "✗ Detailed format missing path label"
    exit 1
fi

if echo "$OUTPUT" | grep -q "Type:"; then
    echo "✓ Detailed format shows type label"
else
    echo "✗ Detailed format missing type label"
    exit 1
fi

if echo "$OUTPUT" | grep -q "Size:"; then
    echo "✓ Detailed format shows size label"
else
    echo "✗ Detailed format missing size label"
    exit 1
fi

if echo "$OUTPUT" | grep -q "Modified:"; then
    echo "✓ Detailed format shows modified time"
else
    echo "✗ Detailed format missing modified time"
    exit 1
fi

# Test 3: JSON format
echo "[Test 3] JSON format output"
OUTPUT=$("$FMF" -r --format json "$TEST_DIR" 2>&1)

# Extract only the JSON part (skip the "Scanning..." line)
JSON_OUTPUT=$(echo "$OUTPUT" | grep -A 9999 "^{")

if echo "$JSON_OUTPUT" | grep -q '"count":.*4'; then
    echo "✓ JSON format has count field"
else
    echo "✗ JSON format missing count field"
    echo "JSON was: $JSON_OUTPUT"
    exit 1
fi

if echo "$JSON_OUTPUT" | grep -q '"files":'; then
    echo "✓ JSON format has files array"
else
    echo "✗ JSON format missing files array"
    exit 1
fi

if echo "$JSON_OUTPUT" | grep -q '"path":'; then
    echo "✓ JSON format includes file path"
else
    echo "✗ JSON format missing file path"
    echo "JSON was: $JSON_OUTPUT"
    exit 1
fi

if echo "$JSON_OUTPUT" | grep -q '"type":'; then
    echo "✓ JSON format includes file type"
else
    echo "✗ JSON format missing file type"
    exit 1
fi

if echo "$JSON_OUTPUT" | grep -q '"size":'; then
    echo "✓ JSON format includes file size"
else
    echo "✗ JSON format missing file size"
    exit 1
fi

# Test 4: Invalid format
echo "[Test 4] Invalid format handling"
if "$FMF" -r --format invalid "$TEST_DIR" 2>&1 | grep -q "Unknown format"; then
    echo "✓ Invalid format shows error message"
else
    echo "✗ Invalid format should show error"
    exit 1
fi

# Test 5: Color option (just check it doesn't crash)
echo "[Test 5] Color options"
"$FMF" -r --color "$TEST_DIR" > /dev/null 2>&1
echo "✓ --color option works without errors"

"$FMF" -r --no-color "$TEST_DIR" > /dev/null 2>&1
echo "✓ --no-color option works without errors"

# Test 6: Empty results
echo "[Test 6] Empty results handling"
OUTPUT=$("$FMF" -r -e .nonexistent "$TEST_DIR" 2>&1)
if echo "$OUTPUT" | grep -q "Found 0 items"; then
    echo "✓ Empty results handled correctly"
else
    echo "✗ Empty results not handled properly"
    exit 1
fi

# Test 7: JSON format for empty results
echo "[Test 7] JSON format for empty results"
JSON_OUTPUT=$("$FMF" -r -e .nonexistent --format json "$TEST_DIR" 2>&1 | grep -A 9999 "^{")
if echo "$JSON_OUTPUT" | grep -q '"count":.*0'; then
    echo "✓ JSON empty results has count 0"
else
    echo "✗ JSON empty results incorrect"
    exit 1
fi

# Cleanup
rm -rf "$TEST_DIR"

echo ""
echo "=== All output format tests passed! ==="
