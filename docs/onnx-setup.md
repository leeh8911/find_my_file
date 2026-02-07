# ONNX Runtime Setup Guide

This guide explains how to install ONNX Runtime and download the all-MiniLM-L6-v2 model for local embedding generation.

## Prerequisites

- C++17 compiler
- CMake 3.14+
- Python 3.7+ (for model conversion)

## Step 1: Install ONNX Runtime

### Option A: Ubuntu/Debian (apt)

```bash
# Install libonnxruntime (if available in your distro)
sudo apt-get update
sudo apt-get install libonnxruntime-dev

# If not available, use Option B or C
```

###Option B: Download Pre-built Binary

```bash
# Download ONNX Runtime (Linux x64, CPU)
cd ~
wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-x64-1.16.3.tgz
tar -xzf onnxruntime-linux-x64-1.16.3.tgz

# Install to user directory
mkdir -p ~/.local/include ~/.local/lib
cp -r onnxruntime-linux-x64-1.16.3/include/* ~/.local/include/
cp -r onnxruntime-linux-x64-1.16.3/lib/* ~/.local/lib/

# Add to LD_LIBRARY_PATH
echo 'export LD_LIBRARY_PATH=$HOME/.local/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

### Option C: Build from Source (Advanced)

```bash
git clone --recursive https://github.com/Microsoft/onnxruntime
cd onnxruntime
./build.sh --config Release --build_shared_lib --parallel

# Install
cd build/Linux/Release
sudo make install
```

## Step 2: Download and Convert Model

### Install Required Python Packages

```bash
pip install transformers onnx onnxruntime optimum torch
```

### Convert all-MiniLM-L6-v2 to ONNX

Create a Python script `convert_model.py`:

```python
#!/usr/bin/env python3
"""
Convert sentence-transformers/all-MiniLM-L6-v2 to ONNX format
"""
from pathlib import Path
from transformers import AutoTokenizer, AutoModel
import torch
import onnx
from onnxruntime.quantization import quantize_dynamic, QuantType

def convert_to_onnx(model_name="sentence-transformers/all-MiniLM-L6-v2"):
    """Convert Hugging Face model to ONNX"""
    
    # Output directory
    output_dir = Path.home() / ".fmf" / "models"
    output_dir.mkdir(parents=True, exist_ok=True)
    
    model_path = output_dir / "all-MiniLM-L6-v2.onnx"
    
    print(f"Loading model: {model_name}")
    tokenizer = AutoTokenizer.from_pretrained(model_name)
    model = AutoModel.from_pretrained(model_name)
    model.eval()
    
    # Dummy input
    text = "This is a sample sentence"
    inputs = tokenizer(text, return_tensors="pt", padding=True, truncation=True, max_length=512)
    
    # Export to ONNX
    print(f"Exporting to ONNX: {model_path}")
    torch.onnx.export(
        model,
        (inputs["input_ids"], inputs["attention_mask"]),
        model_path,
        input_names=["input_ids", "attention_mask"],
        output_names=["last_hidden_state"],
        dynamic_axes={
            "input_ids": {0: "batch", 1: "sequence"},
            "attention_mask": {0: "batch", 1: "sequence"},
            "last_hidden_state": {0: "batch", 1: "sequence"}
        },
        opset_version=14,
        do_constant_folding=True
    )
    
    print(f"✅ Model saved to: {model_path}")
    
    # Verify model
    onnx_model = onnx.load(str(model_path))
    onnx.checker.check_model(onnx_model)
    print("✅ Model verification passed")
    
    # Optional: Quantize for faster inference
    quantized_path = output_dir / "all-MiniLM-L6-v2-quantized.onnx"
    print(f"Quantizing model: {quantized_path}")
    quantize_dynamic(
        model_path,
        quantized_path,
        weight_type=QuantType.QUInt8
    )
    print(f"✅ Quantized model saved: {quantized_path}")
    
    return model_path

if __name__ == "__main__":
    model_path = convert_to_onnx()
    print(f"\n🎉 Conversion complete!")
    print(f"Model path: {model_path}")
    print(f"Use with: -DENABLE_LOCAL_EMBEDDING=ON")
```

Run the conversion:

```bash
chmod +x convert_model.py
./convert_model.py
```

### Alternative: Download Pre-converted ONNX Model

```bash
# Create model directory
mkdir -p ~/.fmf/models

# Download from Optimum repository (if available)
# Or use Hugging Face Optimum CLI
optimum-cli export onnx --model sentence-transformers/all-MiniLM-L6-v2 ~/.fmf/models/
```

## Step 3: Build with ONNX Runtime

```bash
cd /path/to/find_my_files
mkdir -p build && cd build

# Configure with ONNX Runtime enabled
cmake .. -DENABLE_LOCAL_EMBEDDING=ON

# Build
make -j4

# Verify ONNX Runtime was found
# Check CMake output for: "ONNX Runtime found"
```

## Step 4: Test

```bash
# Run tests
./tests --gtest_filter="LocalEmbeddingProviderTest.*"

# If all pass, ONNX integration is working!
```

## Step 5: Usage

### C++ API

```cpp
#include "infrastructure/ai/local_embedding_provider.h"

// Create provider with model path
fmf::LocalEmbeddingProvider provider("all-MiniLM-L6-v2.onnx");

// Generate embedding
auto embedding = provider.generateEmbedding("This is a test sentence");
std::cout << "Embedding dimension: " << embedding.size() << std::endl;

// Batch generation
std::vector<std::string> texts = {"Hello", "World", "Test"};
auto embeddings = provider.batchGenerate(texts);
```

### Command Line (Future)

```bash
# Index files with local embedding
./find_my_files --index ~/documents --embedding-provider local

# Search semantically
./find_my_files --semantic-search "machine learning tutorial" ~/documents
```

## Troubleshooting

### ONNX Runtime not found

- Check paths in CMake output
- Ensure `libonnxruntime.so` is in `LD_LIBRARY_PATH`
- Try: `export LD_LIBRARY_PATH=$HOME/.local/lib:$LD_LIBRARY_PATH`

### Model not found

- Check `~/.fmf/models/all-MiniLM-L6-v2.onnx` exists
- Re-run conversion script
- Check file permissions

### Inference errors

- Verify model opset version (14 recommended)
- Check input tensor shapes (batch_size, seq_length)
- Enable debug logging: `ORT_LOGGING_LEVEL_INFO`

### Performance issues

- Use quantized model for faster inference
- Enable OpenMP: `export OMP_NUM_THREADS=4`
- Consider GPU acceleration (requires CUDA build)

## Model Details

**all-MiniLM-L6-v2**
- Source: sentence-transformers/all-MiniLM-L6-v2
- Dimension: 384
- Max tokens: 512
- Size: ~80MB (ONNX)
- Speed: ~1000 sentences/sec (CPU)
- Use case: General-purpose semantic search

## Advanced: GPU Acceleration

```bash
# Build ONNX Runtime with CUDA
# Requires CUDA Toolkit 11.x+

cmake .. -DENABLE_LOCAL_EMBEDDING=ON -DUSE_CUDA=ON
make -j4
```

```cpp
// Enable GPU in code
fmf::LocalEmbeddingProvider provider("all-MiniLM-L6-v2.onnx", 
                                      /*useGpu=*/true);
```

## References

- [ONNX Runtime](https://onnxruntime.ai/)
- [sentence-transformers](https://www.sbert.net/)
- [Hugging Face all-MiniLM-L6-v2](https://huggingface.co/sentence-transformers/all-MiniLM-L6-v2)
- [Optimum](https://huggingface.co/docs/optimum/index)
