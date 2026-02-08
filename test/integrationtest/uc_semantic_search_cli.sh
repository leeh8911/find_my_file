#!/bin/bash
# Integration test: Semantic search CLI

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/../../build"
FMF_BIN="$BUILD_DIR/find_my_files"
TEST_DATA_DIR="$SCRIPT_DIR/../testdata"
SEM_DIR="$TEST_DATA_DIR/semantic_cli"
MODEL_PATH="$HOME/.fmf/models/all-MiniLM-L6-v2.onnx"

GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m'

echo "=== UC: Semantic Search CLI ==="

if [ ! -f "$FMF_BIN" ]; then
    echo -e "${YELLOW}⚠ find_my_files binary not found, skipping${NC}"
    exit 0
fi

if [ ! -f "$MODEL_PATH" ] && [ -z "$FMF_EMBEDDING_MODEL_URL" ]; then
    echo -e "${YELLOW}⚠ Embedding model missing and FMF_EMBEDDING_MODEL_URL not set${NC}"
    exit 0
fi

mkdir -p "$SEM_DIR"
cat > "$SEM_DIR/ai.txt" << 'EOF'
Artificial intelligence and neural networks are core topics in machine learning.
EOF
cat > "$SEM_DIR/cook.txt" << 'EOF'
Boil pasta with salt and add tomato sauce.
EOF

OUTPUT=$($FMF_BIN --semantic-search "artificial intelligence" --top-k 1 "$SEM_DIR" 2>/dev/null | head -1)

if echo "$OUTPUT" | grep -q "ai.txt"; then
    echo -e "${GREEN}✓ Semantic search returns AI file${NC}"
else
    echo -e "${RED}✗ Semantic search did not return AI file${NC}"
    exit 1
fi

rm -rf "$SEM_DIR"

echo -e "${GREEN}=== Semantic search CLI tests passed ===${NC}"
