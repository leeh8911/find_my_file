# Quick Start: Semantic Search with all-MiniLM-L6-v2

This guide helps you quickly set up and test semantic search with the real all-MiniLM-L6-v2 model.

## Prerequisites

```bash
# Check you're on the correct branch
git branch --show-current  # Should be: phase/semantic-search

# Install Python dependencies
pip install torch transformers onnx optimum
```

## Step 1: Download Model (Automatic)

The model will be automatically downloaded when you run the integration test:

```bash
# Run integration test - model downloads automatically
./test/integrationtest/uc_semantic_search.sh
```

Or download manually:

```bash
# Manual download
python3 scripts/download_model.py

# Verify model exists
ls -lh ~/.fmf/models/all-MiniLM-L6-v2.onnx
```

## Step 2: Run Integration Tests

```bash
# If ONNX Runtime is enabled
cd build
cmake .. -DENABLE_LOCAL_EMBEDDING=ON
make -j4

# Run semantic search tests
cd ..
./test/integrationtest/uc_semantic_search.sh
```

## Expected Output

```
=== UC: Semantic Search Integration Test ===
Checking Python dependencies...
✓ Python dependencies available

Downloading all-MiniLM-L6-v2 model...
✓ Model ready

=== Running with REAL all-MiniLM-L6-v2 model ===

Test 1: LocalEmbeddingProvider initialization
✓ Test 1 passed: Provider initialization

Test 2: Embedding generation consistency
✓ Test 2 passed: Embedding consistency
  (Using real ONNX model)

Test 3: Batch processing
✓ Test 3 passed: Batch processing

Test 4: SemanticSearcher integration
✓ Test 4 passed: SemanticSearcher integration
  Query: 'artificial intelligence neural networks'
  Expected: ai_basics.txt and ml_tutorial.txt ranked higher

Test 5: Similar file search
✓ Test 5 passed: Similar file search
  Target: ml_tutorial.txt
  Expected: ai_basics.txt (similar AI content)

=== Semantic Search Integration Tests Complete (REAL MODEL) ===
Model: all-MiniLM-L6-v2 (384 dimensions)
```

## What Gets Tested

1. **Model Download**: Automatic download from Hugging Face
2. **ONNX Conversion**: PyTorch → ONNX format
3. **Provider Init**: LocalEmbeddingProvider with real model
4. **Embedding Generation**: Consistent 384-dim vectors
5. **Batch Processing**: Multiple texts at once
6. **Semantic Search**: Natural language queries
7. **Similar Files**: Find documents with similar content

## Test Data

The test creates sample documents:

- **ml_tutorial.txt**: Machine learning content
- **ai_basics.txt**: Artificial intelligence overview
- **cooking_recipe.txt**: Pasta recipe
- **travel_tips.txt**: Travel advice

Query: "artificial intelligence neural networks"  
Expected: ai_basics.txt and ml_tutorial.txt rank higher than cooking/travel

## Troubleshooting

### No ONNX Runtime

```
⚠ ONNX Runtime not enabled, skipping semantic search test
  To enable: cmake .. -DENABLE_LOCAL_EMBEDDING=ON
```

**Solution**: See [docs/onnx-setup.md](onnx-setup.md) for ONNX Runtime installation.

### Python Dependencies Missing

```
⚠ Python dependencies not available
  Required: torch, transformers, onnx
```

**Solution**:
```bash
pip install torch transformers onnx optimum
```

### Model Download Fails

```
✗ Model download failed
  Falling back to placeholder tests...
```

**Solution**:
- Check internet connection
- Retry: `python3 scripts/download_model.py`
- Manual download from Hugging Face

### Tests Pass with Placeholder

```
=== Running with PLACEHOLDER model (structure tests only) ===
```

This means Python dependencies are missing, but basic structure tests still run.

## Performance

With real model:
- Model size: ~80MB
- Download time: 1-2 minutes (first run only)
- Inference speed: ~1000 sentences/sec (CPU)
- Test duration: ~5-10 seconds

## What's Next

After tests pass:
1. Test with your own documents
2. Tune chunking strategies
3. Benchmark performance
4. Enable ONNX Runtime optimizations

## Manual Testing

```bash
# Build test helper manually
cd build
g++ -std=c++17 -I../include \
    -o test_embedding \
    ../src/infrastructure/ai/local_embedding_provider.cpp \
    ../src/infrastructure/ai/simple_tokenizer.cpp \
    -DENABLE_ONNX_RUNTIME \
    -lonnxruntime

# Test embedding generation
./test_embedding ~/.fmf/models/all-MiniLM-L6-v2.onnx "Hello world"
```

## Resources

- Model: [sentence-transformers/all-MiniLM-L6-v2](https://huggingface.co/sentence-transformers/all-MiniLM-L6-v2)
- ONNX Setup: [docs/onnx-setup.md](onnx-setup.md)
- Architecture: [docs/architecture.md](architecture.md)
