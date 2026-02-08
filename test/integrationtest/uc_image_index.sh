#!/bin/bash
# Integration test: Image indexing pipeline

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/../../build"
FMF_BIN="$BUILD_DIR/find_my_files"
TEST_DATA_DIR="$SCRIPT_DIR/../testdata"
TEST_IMAGES="$TEST_DATA_DIR/images"
INDEX_DB="$TEST_DATA_DIR/image_index_test.db"
EMBED_MODEL="$HOME/.fmf/models/all-MiniLM-L6-v2.onnx"

GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m'

echo "=== UC: Image Index Pipeline ==="

if [ ! -f "$FMF_BIN" ]; then
    echo -e "${YELLOW}⚠ find_my_files binary not found, skipping${NC}"
    exit 0
fi

if ! command -v tesseract >/dev/null 2>&1; then
    echo -e "${YELLOW}⚠ tesseract not available, skipping${NC}"
    exit 0
fi

CAPTION_CMD="${FMF_CAPTION_CMD:-blip-caption}"
if ! command -v "$CAPTION_CMD" >/dev/null 2>&1; then
    echo -e "${YELLOW}⚠ Caption command not available: $CAPTION_CMD${NC}"
    exit 0
fi

if [ ! -f "$EMBED_MODEL" ] && [ -z "$FMF_EMBEDDING_MODEL_URL" ]; then
    echo -e "${YELLOW}⚠ Embedding model missing and FMF_EMBEDDING_MODEL_URL not set${NC}"
    exit 0
fi

rm -f "$INDEX_DB"

OUTPUT=$($FMF_BIN index-image --path "$TEST_IMAGES" --out "$INDEX_DB" 2>&1)

if echo "$OUTPUT" | grep -q "Image indexing support is not enabled"; then
    echo -e "${YELLOW}⚠ Image indexing not enabled, skipping${NC}"
    exit 0
fi

if echo "$OUTPUT" | grep -q "OCR_TEXT:"; then
    echo -e "${GREEN}✓ OCR text printed${NC}"
else
    echo -e "${RED}✗ OCR text missing${NC}"
    exit 1
fi

if echo "$OUTPUT" | grep -q "CAPTION_TEXT:"; then
    echo -e "${GREEN}✓ Caption text printed${NC}"
else
    echo -e "${RED}✗ Caption text missing${NC}"
    exit 1
fi

if echo "$OUTPUT" | grep -q "INDEX_TEXT_LEN:"; then
    echo -e "${GREEN}✓ Index text length printed${NC}"
else
    echo -e "${RED}✗ Index text length missing${NC}"
    exit 1
fi

if echo "$OUTPUT" | grep -q "EMBED_DIM:"; then
    echo -e "${GREEN}✓ Embedding dimension printed${NC}"
else
    echo -e "${RED}✗ Embedding dimension missing${NC}"
    exit 1
fi

if [ ! -s "$INDEX_DB" ]; then
    echo -e "${RED}✗ Index DB not created${NC}"
    exit 1
fi

if echo "$OUTPUT" | grep -q "PREVIEW_OCR:" && echo "$OUTPUT" | grep -q "receipt_en.png"; then
    echo -e "${GREEN}✓ OCR preview includes receipt_en.png${NC}"
else
    echo -e "${RED}✗ OCR preview missing receipt_en.png${NC}"
    exit 1
fi

if echo "$OUTPUT" | grep -q "PREVIEW_CAPTION:" && echo "$OUTPUT" | grep -q "house_scene.png"; then
    echo -e "${GREEN}✓ Caption preview includes house_scene.png${NC}"
else
    echo -e "${RED}✗ Caption preview missing house_scene.png${NC}"
    exit 1
fi

rm -f "$INDEX_DB"

echo -e "${GREEN}=== Image indexing tests passed ===${NC}"
