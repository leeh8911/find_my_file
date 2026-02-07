#!/usr/bin/env python3
"""
Download and convert sentence-transformers/all-MiniLM-L6-v2 to ONNX format
for use in find_my_files semantic search integration tests.
"""

import os
import sys
from pathlib import Path

def check_dependencies():
    """Check if required Python packages are installed"""
    try:
        import torch
        import transformers
        import onnx
        return True
    except ImportError as e:
        print(f"Error: Missing required package: {e.name}")
        print("\nPlease install required packages:")
        print("  pip install torch transformers onnx optimum")
        return False

def convert_model():
    """Download and convert model to ONNX"""
    import torch
    from transformers import AutoTokenizer, AutoModel
    import onnx
    
    model_name = "sentence-transformers/all-MiniLM-L6-v2"
    
    # Create output directory
    output_dir = Path.home() / ".fmf" / "models"
    output_dir.mkdir(parents=True, exist_ok=True)
    
    model_path = output_dir / "all-MiniLM-L6-v2.onnx"
    
    # Check if model already exists
    if model_path.exists():
        print(f"✓ Model already exists: {model_path}")
        
        # Verify it's valid
        try:
            onnx_model = onnx.load(str(model_path))
            onnx.checker.check_model(onnx_model)
            print("✓ Model verification passed")
            return str(model_path)
        except Exception as e:
            print(f"⚠ Existing model is invalid: {e}")
            print("  Re-downloading...")
    
    print(f"Downloading model: {model_name}")
    print("  This may take a few minutes on first run...")
    
    tokenizer = AutoTokenizer.from_pretrained(model_name)
    model = AutoModel.from_pretrained(model_name)
    model.eval()
    
    # Create dummy input
    text = "This is a sample sentence"
    inputs = tokenizer(text, return_tensors="pt", padding=True, truncation=True, max_length=512)
    
    print(f"Exporting to ONNX: {model_path}")
    
    with torch.no_grad():
        torch.onnx.export(
            model,
            (inputs["input_ids"], inputs["attention_mask"]),
            str(model_path),
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
    
    # Verify exported model
    onnx_model = onnx.load(str(model_path))
    onnx.checker.check_model(onnx_model)
    
    print(f"✓ Model saved to: {model_path}")
    print(f"✓ Model verification passed")
    
    return str(model_path)

if __name__ == "__main__":
    print("=== all-MiniLM-L6-v2 ONNX Converter ===\n")
    
    if not check_dependencies():
        sys.exit(1)
    
    try:
        model_path = convert_model()
        print(f"\n🎉 Success! Model ready at:")
        print(f"   {model_path}")
        print(f"\nYou can now run semantic search integration tests.")
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
