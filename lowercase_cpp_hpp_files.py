#!/usr/bin/env python3
"""
Script to convert all .hpp and .cpp filenames to lowercase in the hydra_sdk directory.
This script will:
1. Find all .hpp and .cpp files (excluding lib/ directory)
2. Convert their filenames to lowercase
3. Report the changes made
4. Can revert changes in specified directories

Handles case-insensitive filesystems by using a temporary filename during renaming.
"""

import os
import sys
import uuid
import shutil
from pathlib import Path

def convert_to_lowercase(directory, exclude_dirs=None, revert_dirs=None):
    """
    Recursively find all .hpp and .cpp files in the given directory
    and convert their filenames to lowercase.
    
    Args:
        directory: The root directory to start the search from
        exclude_dirs: List of directory paths to exclude from conversion
        revert_dirs: List of directory paths where to revert previous conversions
    
    Returns:
        tuple: (renamed_files, skipped_files, reverted_files) - Lists of renamed, skipped, and reverted files
    """
    if exclude_dirs is None:
        exclude_dirs = []
    if revert_dirs is None:
        revert_dirs = []
    renamed_files = []
    skipped_files = []
    reverted_files = []
    
    # Process revert directories first if specified
    if revert_dirs:
        for revert_dir in revert_dirs:
            print(f"Reverting changes in {revert_dir}...")
            reverted_files.extend(revert_lowercase_conversion(revert_dir))
    
    # First pass: collect all files that need to be renamed
    files_to_rename = []
    for root, _, files in os.walk(directory):
        # Skip excluded directories
        should_exclude = False
        for exclude_dir in exclude_dirs:
            if root.startswith(exclude_dir):
                should_exclude = True
                break
        if should_exclude:
            continue
            
        for filename in files:
            if filename.lower().endswith(('.cpp', '.hpp')):
                filepath = os.path.join(root, filename)
                lowercase_filename = filename.lower()
                lowercase_filepath = os.path.join(root, lowercase_filename)
                
                if filename != lowercase_filename:
                    files_to_rename.append((filepath, lowercase_filepath))
    
    # Second pass: rename files using a temporary name first to avoid conflicts
    for filepath, lowercase_filepath in files_to_rename:
        try:
            # Generate a temporary filename with a UUID to avoid conflicts
            temp_filename = f"temp_{uuid.uuid4().hex}{os.path.splitext(filepath)[1]}"
            temp_filepath = os.path.join(os.path.dirname(filepath), temp_filename)
            
            # Step 1: Rename to temporary filename
            os.rename(filepath, temp_filepath)
            
            # Step 2: If lowercase file exists, back it up
            backup_filepath = None
            if os.path.exists(lowercase_filepath):
                backup_filename = f"backup_{uuid.uuid4().hex}{os.path.splitext(lowercase_filepath)[1]}"
                backup_filepath = os.path.join(os.path.dirname(lowercase_filepath), backup_filename)
                os.rename(lowercase_filepath, backup_filepath)
            
            # Step 3: Rename temporary file to lowercase
            os.rename(temp_filepath, lowercase_filepath)
            
            # Step 4: If we had a backup, compare contents and keep newer file
            if backup_filepath:
                # If files are identical, remove the backup
                if os.path.getsize(backup_filepath) == os.path.getsize(lowercase_filepath):
                    with open(backup_filepath, 'rb') as f1, open(lowercase_filepath, 'rb') as f2:
                        if f1.read() == f2.read():
                            os.remove(backup_filepath)
                            print(f"Renamed: {filepath} -> {lowercase_filepath} (identical to existing file)")
                            renamed_files.append((filepath, lowercase_filepath))
                            continue
                
                # Files differ, keep both with different names
                alt_filename = f"{os.path.splitext(lowercase_filename)[0]}_alt{os.path.splitext(lowercase_filename)[1]}"
                alt_filepath = os.path.join(os.path.dirname(lowercase_filepath), alt_filename)
                os.rename(backup_filepath, alt_filepath)
                print(f"Renamed: {filepath} -> {lowercase_filepath} (kept existing as {alt_filename})")
            else:
                print(f"Renamed: {filepath} -> {lowercase_filepath}")
            
            renamed_files.append((filepath, lowercase_filepath))
            
        except Exception as e:
            print(f"ERROR: Failed to rename {filepath}: {str(e)}")
            skipped_files.append((filepath, str(e)))
            
            # Try to restore original state if possible
            if 'temp_filepath' in locals() and os.path.exists(temp_filepath):
                try:
                    os.rename(temp_filepath, filepath)
                except:
                    pass
    
    return renamed_files, skipped_files, reverted_files

def revert_lowercase_conversion(directory):
    """
    Find all lowercase .cpp and .hpp files in the given directory that might have been
    converted from uppercase and revert them to their original names if possible.
    
    Args:
        directory: The directory to process
        
    Returns:
        list: List of files that were reverted
    """
    reverted_files = []
    
    # Map of lowercase -> potential uppercase names
    case_map = {
        # Common C++ naming patterns
        'cpp': ['CPP', 'Cpp'],
        'hpp': ['HPP', 'Hpp'],
    }
    
    # Common capitalization patterns for filenames
    capitalization_patterns = [
        # Original: first letter uppercase, rest lowercase
        lambda s: s[0].upper() + s[1:] if s else s,
        # Original: all uppercase
        lambda s: s.upper(),
        # Original: camelCase (first word lowercase, subsequent words uppercase)
        lambda s: ''.join(word if i == 0 else word.capitalize() for i, word in enumerate(s.split('_'))),
        # Original: PascalCase (all words capitalized)
        lambda s: ''.join(word.capitalize() for word in s.split('_')),
    ]
    
    for root, _, files in os.walk(directory):
        for filename in files:
            if filename.lower().endswith(('.cpp', '.hpp')):
                filepath = os.path.join(root, filename)
                
                # Skip files that are not all lowercase
                if filename != filename.lower():
                    continue
                    
                name, ext = os.path.splitext(filename)
                ext = ext[1:]  # Remove the dot
                
                # Try to find original capitalization
                potential_original_names = []
                
                # Try different capitalization patterns for the name
                for pattern in capitalization_patterns:
                    potential_name = pattern(name)
                    if potential_name != name:
                        # Try different capitalization patterns for the extension
                        for potential_ext in case_map.get(ext.lower(), [ext.upper(), ext.capitalize()]):
                            potential_original_names.append(f"{potential_name}.{potential_ext}")
                
                # Also add the exact original extension cases
                potential_original_names.append(f"{name.upper()}.{ext}")
                potential_original_names.append(f"{name.capitalize()}.{ext}")
                
                # Remove duplicates
                potential_original_names = list(set(potential_original_names))
                
                # Try to find any files in the same directory with similar names
                similar_files = []
                for root_dir, _, dir_files in os.walk(root):
                    if root_dir != root:
                        continue
                    for dir_file in dir_files:
                        if dir_file.lower() == filename.lower() and dir_file != filename:
                            similar_files.append(dir_file)
                
                if similar_files:
                    # Use the similar file as the original name
                    original_name = similar_files[0]
                    original_path = os.path.join(root, original_name)
                    
                    # Generate a temporary filename
                    temp_filename = f"temp_{uuid.uuid4().hex}{os.path.splitext(filepath)[1]}"
                    temp_filepath = os.path.join(root, temp_filename)
                    
                    try:
                        # Rename to temp file first
                        os.rename(filepath, temp_filepath)
                        # Then rename to the original capitalization
                        os.rename(temp_filepath, original_path)
                        reverted_files.append((filepath, original_path))
                        print(f"Reverted: {filepath} -> {original_path}")
                    except Exception as e:
                        print(f"ERROR: Failed to revert {filepath}: {str(e)}")
                        # Try to restore if possible
                        if os.path.exists(temp_filepath):
                            try:
                                os.rename(temp_filepath, filepath)
                            except:
                                pass
                elif potential_original_names:
                    # Try each potential original name
                    for original_name in potential_original_names:
                        original_path = os.path.join(root, original_name)
                        
                        # Skip if the original path already exists
                        if os.path.exists(original_path):
                            continue
                            
                        # Generate a temporary filename
                        temp_filename = f"temp_{uuid.uuid4().hex}{os.path.splitext(filepath)[1]}"
                        temp_filepath = os.path.join(root, temp_filename)
                        
                        try:
                            # Rename to temp file first
                            os.rename(filepath, temp_filepath)
                            # Then rename to the potential original capitalization
                            os.rename(temp_filepath, original_path)
                            reverted_files.append((filepath, original_path))
                            print(f"Reverted: {filepath} -> {original_path} (best guess)")
                            break
                        except Exception as e:
                            print(f"ERROR: Failed to revert {filepath} to {original_path}: {str(e)}")
                            # Try to restore if possible
                            if os.path.exists(temp_filepath):
                                try:
                                    os.rename(temp_filepath, filepath)
                                except:
                                    pass
    
    return reverted_files

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <directory> [--exclude dir1,dir2,...] [--revert dir1,dir2,...]")
        print(f"Example: {sys.argv[0]} /Volumes/BIGCODE/hydra_sdk --exclude /Volumes/BIGCODE/hydra_sdk/lib")
        print(f"Example: {sys.argv[0]} /Volumes/BIGCODE/hydra_sdk --revert /Volumes/BIGCODE/hydra_sdk/lib")
        sys.exit(1)
    
    directory = sys.argv[1]
    if not os.path.isdir(directory):
        print(f"ERROR: {directory} is not a valid directory")
        sys.exit(1)
    
    exclude_dirs = []
    revert_dirs = []
    
    # Parse command line arguments
    i = 2
    while i < len(sys.argv):
        if sys.argv[i] == "--exclude" and i + 1 < len(sys.argv):
            exclude_dirs = sys.argv[i + 1].split(',')
            i += 2
        elif sys.argv[i] == "--revert" and i + 1 < len(sys.argv):
            revert_dirs = sys.argv[i + 1].split(',')
            i += 2
        else:
            print(f"ERROR: Unknown argument {sys.argv[i]}")
            sys.exit(1)
    
    # Default exclude lib/ if not specified and not reverting
    if not exclude_dirs and not revert_dirs:
        lib_dir = os.path.join(directory, "lib")
        if os.path.isdir(lib_dir):
            exclude_dirs = [lib_dir]
            print(f"Note: Automatically excluding {lib_dir}")
    
    if revert_dirs:
        print(f"Reverting .cpp and .hpp filename conversions in: {', '.join(revert_dirs)}")
    else:
        print(f"Converting .cpp and .hpp filenames to lowercase in {directory}")
        if exclude_dirs:
            print(f"Excluding directories: {', '.join(exclude_dirs)}")
    
    print("=" * 80)
    
    renamed_files, skipped_files, reverted_files = convert_to_lowercase(directory, exclude_dirs, revert_dirs)
    
    print("\nSummary:")
    print(f"- Total files renamed: {len(renamed_files)}")
    print(f"- Total files skipped: {len(skipped_files)}")
    print(f"- Total files reverted: {len(reverted_files)}")
    
    if renamed_files:
        print("\nRenamed files:")
        for old, new in renamed_files:
            print(f"  {old} -> {new}")
    
    if skipped_files:
        print("\nSkipped files:")
        for file, reason in skipped_files:
            print(f"  {file} - Reason: {reason}")
            
    if reverted_files:
        print("\nReverted files:")
        for old, new in reverted_files:
            print(f"  {old} -> {new}")

if __name__ == "__main__":
    main()
