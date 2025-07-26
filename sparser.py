import torch
import os
import argparse

def convert_tensor_to_sparse(file_path):
    try:
        tensor = torch.load(file_path)

        if isinstance(tensor, torch.Tensor) and not tensor.is_sparse:
            sparse_tensor = tensor.to_sparse()
            torch.save(sparse_tensor, file_path)
            print(f"Converted to sparse: {file_path}")
        else:
            print(f"Skipped (already sparse or not a tensor): {file_path}")

        # Cleanup to free memory
        del tensor
        if 'sparse_tensor' in locals():
            del sparse_tensor
        torch.cuda.empty_cache()  # No-op on CPU, safe to call always

    except Exception as e:
        print(f"Error processing {file_path}: {e}")


def convert_directory_to_sparse(directory):
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(".pt") and file != "model.pt":
                full_path = os.path.join(root, file)
                convert_tensor_to_sparse(full_path)
                print(f'converted {full_path}')

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert .pt tensors to sparse format.")
    parser.add_argument("directory", help="Directory to search for .pt files")
    args = parser.parse_args()

    convert_directory_to_sparse(args.directory)
