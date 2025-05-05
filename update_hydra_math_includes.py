#!/usr/bin/env python3
"""
Script to update include statements in files that reference capitalized versions
of hydra_math headers. This script will:
1. Find all files that include hydra_math headers with capitalized filenames
2. Update those include statements to use lowercase filenames
3. Report the changes made
"""

import os
import sys
import re
from pathlib import Path

def update_includes(directory, dry_run=False):
    """
    Find all files that include hydra_math headers with capitalized filenames
    and update those include statements to use lowercase filenames.
    
    Args:
        directory: The root directory to start the search from
        dry_run: If True, only print changes without modifying files
    
    Returns:
        tuple: (updated_files, skipped_files) - Lists of updated and skipped files
    """
    updated_files = []
    skipped_files = []
    
    # Map of capitalized header names to lowercase versions
    header_map = {
        "BigInt.hpp": "bigint.hpp",
        "BKZ.hpp": "bkz.hpp",
        "ComplexMatrix.hpp": "complexmatrix.hpp",
        "GaloisVector.hpp": "galoisvector.hpp",
        "Huffman.hpp": "huffman.hpp",
        "LatticeUtils.hpp": "latticeutils.hpp",
        "LatticeSolver.hpp": "latticesolver.hpp",
        "LLL.hpp": "lll.hpp",
        "MatrixUtils.hpp": "matrixutils.hpp",
        "Modular.hpp": "modular.hpp",
        "Pedersen.hpp": "pedersen.hpp",
        "Rational.hpp": "rational.hpp",
        "Shamir.hpp": "shamir.hpp"
    }
    
    # Regular expression to match include statements for hydra_math headers
    include_pattern = re.compile(r'#include\s+([<"])hydra_math/([A-Za-z0-9_]+\.hpp)([>"])')
    
    # Find all source and header files
    for root, _, files in os.walk(directory):
        # Skip lib directory
        if "/lib/" in root or "\\lib\\" in root:
            continue
            
        for filename in files:
            if filename.lower().endswith(('.cpp', '.hpp', '.h', '.cc', '.c', '.cxx')):
                filepath = os.path.join(root, filename)
                
                try:
                    # Read file content
                    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                    
                    # Check if file contains any hydra_math includes
                    if "hydra_math/" in content:
                        # Replace capitalized header names with lowercase
                        modified_content = content
                        changes_made = False
                        
                        # Find all include statements
                        for match in include_pattern.finditer(content):
                            quote_type = match.group(1)
                            header_name = match.group(2)
                            quote_end = match.group(3)
                            
                            # Check if header name is in our map
                            if header_name in header_map:
                                old_include = f'#include {quote_type}hydra_math/{header_name}{quote_end}'
                                new_include = f'#include {quote_type}hydra_math/{header_map[header_name]}{quote_end}'
                                
                                if old_include in modified_content:
                                    modified_content = modified_content.replace(old_include, new_include)
                                    changes_made = True
                                    print(f"In {filepath}:")
                                    print(f"  {old_include} -> {new_include}")
                        
                        # Write changes to file if any were made
                        if changes_made:
                            if not dry_run:
                                with open(filepath, 'w', encoding='utf-8') as f:
                                    f.write(modified_content)
                                updated_files.append(filepath)
                            else:
                                print(f"[DRY RUN] Would update: {filepath}")
                                updated_files.append(filepath)
                
                except Exception as e:
                    print(f"ERROR: Failed to process {filepath}: {str(e)}")
                    skipped_files.append((filepath, str(e)))
    
    return updated_files, skipped_files

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <directory> [--dry-run]")
        print(f"Example: {sys.argv[0]} /Volumes/BIGCODE/hydra_sdk")
        print(f"Example: {sys.argv[0]} /Volumes/BIGCODE/hydra_sdk --dry-run")
        sys.exit(1)
    
    directory = sys.argv[1]
    dry_run = "--dry-run" in sys.argv
    
    if not os.path.isdir(directory):
        print(f"ERROR: {directory} is not a valid directory")
        sys.exit(1)
    
    print(f"Updating hydra_math include statements in {directory}")
    if dry_run:
        print("DRY RUN MODE: No files will be modified")
    print("=" * 80)
    
    updated_files, skipped_files = update_includes(directory, dry_run)
    
    print("\nSummary:")
    print(f"- Total files updated: {len(updated_files)}")
    print(f"- Total files skipped: {len(skipped_files)}")
    
    if skipped_files:
        print("\nSkipped files:")
        for file, reason in skipped_files:
            print(f"  {file} - Reason: {reason}")

if __name__ == "__main__":
    main()
