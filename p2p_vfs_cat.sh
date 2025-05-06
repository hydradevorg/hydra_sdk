#!/bin/bash

# Check if a file path is provided
if [ $# -lt 1 ]; then
    echo "Usage: $0 <file_path>"
    exit 1
fi

# Get the file path
FILE_PATH=$1

# Run the cat example
./build/examples/p2p_vfs_cat_example/p2p_vfs_cat_example "$FILE_PATH"
