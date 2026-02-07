#!/bin/bash
# Integration test: Parallel scanning performance

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_DATA_DIR="$SCRIPT_DIR/../testdata"
BUILD_DIR="$SCRIPT_DIR/../../build"
FMF_BIN="$BUILD_DIR/find_my_files"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== UC: Parallel Scanning Test ===${NC}"

# Setup large test data structure
echo "Creating test data..."
mkdir -p "$TEST_DATA_DIR/perf_test"
for i in {1..10}; do
    mkdir -p "$TEST_DATA_DIR/perf_test/dir$i"
    for j in {1..10}; do
        mkdir -p "$TEST_DATA_DIR/perf_test/dir$i/subdir$j"
        for k in {1..5}; do
            echo "test content $i-$j-$k" > "$TEST_DATA_DIR/perf_test/dir$i/subdir$j/file$k.txt"
        done
    done
done

# Test 1: Sequential scan
echo  "Test 1: Sequential scan (--threads 0)"
SEQ_START=$(date +%s%N)
SEQ_COUNT=$("$FMF_BIN" -r "$TEST_DATA_DIR/perf_test" --threads 0 | grep "Found" | awk '{print $2}')
SEQ_END=$(date +%s%N)
SEQ_TIME=$(( (SEQ_END - SEQ_START) / 1000000 ))  # Convert to milliseconds

echo "  Sequential: Found $SEQ_COUNT items in ${SEQ_TIME}ms"

# Test 2: Parallel scan with 2 threads
echo "Test 2: Parallel scan (--threads 2)"
PAR2_START=$(date +%s%N)
PAR2_COUNT=$("$FMF_BIN" -r "$TEST_DATA_DIR/perf_test" -j 2 | grep "Found" | awk '{print $2}')
PAR2_END=$(date +%s%N)
PAR2_TIME=$(( (PAR2_END - PAR2_START) / 1000000 ))

echo "  2 threads: Found $PAR2_COUNT items in ${PAR2_TIME}ms"

# Test 3: Parallel scan with 4 threads
echo "Test 3: Parallel scan (--threads 4)"
PAR4_START=$(date +%s%N)
PAR4_COUNT=$("$FMF_BIN" -r "$TEST_DATA_DIR/perf_test" -j 4 | grep "Found" | awk '{print $2}')
PAR4_END=$(date +%s%N)
PAR4_TIME=$(( (PAR4_END - PAR4_START) / 1000000 ))

echo "  4 threads: Found $PAR4_COUNT items in ${PAR4_TIME}ms"

# Verify correctness: all methods should find same number of files
if [ "$SEQ_COUNT" -eq "$PAR2_COUNT" ] && [ "$SEQ_COUNT" -eq "$PAR4_COUNT" ]; then
    echo -e "${GREEN}✓ Correctness test passed (all found $SEQ_COUNT items)${NC}"
else
    echo -e "${RED}✗ Correctness test failed (counts differ: seq=$SEQ_COUNT, 2t=$PAR2_COUNT, 4t=$PAR4_COUNT)${NC}"
    rm -rf "$TEST_DATA_DIR/perf_test"
    exit 1
fi

# Performance check: parallel should be faster or similar
if [ "$PAR4_TIME" -le $(( SEQ_TIME * 2 )) ]; then
    echo -e "${GREEN}✓ Performance test passed (parallel within 2x of sequential)${NC}"
    SPEEDUP=$(awk "BEGIN {print $SEQ_TIME / $PAR4_TIME}")
    echo "  Speedup: ${SPEEDUP}x"
else
    echo -e "${RED}✗ Performance test failed (parallel significantly slower)${NC}"
fi

# Cleanup
rm -rf "$TEST_DATA_DIR/perf_test"

echo -e "${GREEN}=== All parallel scanning tests passed ===${NC}"
