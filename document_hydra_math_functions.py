#!/usr/bin/env python3
"""
Script to add documentation to undocumented functions in hydra_math files.
This script will:
1. Find all .hpp and .cpp files in the hydra_math directory
2. Identify functions without documentation
3. Add appropriate documentation comments
4. Report the changes made
"""

import os
import sys
import re
from pathlib import Path

def document_functions(directory, dry_run=False):
    """
    Find all .hpp and .cpp files in the hydra_math directory and add
    documentation to undocumented functions.
    
    Args:
        directory: The root directory to start the search from
        dry_run: If True, only print changes without modifying files
    
    Returns:
        tuple: (updated_files, skipped_files) - Lists of updated and skipped files
    """
    updated_files = []
    skipped_files = []
    
    # Function to generate documentation for a function
    def generate_documentation(func_name, params, return_type):
        """Generate documentation comment for a function."""
        doc = "/**\n"
        doc += f" * @brief {func_name}\n"
        doc += " *\n"
        
        # Add parameter documentation
        for param in params:
            param_name = param.strip().split()[-1].replace("&", "").replace("*", "")
            doc += f" * @param {param_name} Description of {param_name}\n"
        
        # Add return documentation if not void
        if return_type and return_type.strip() != "void":
            doc += " *\n"
            doc += " * @return Description of return value\n"
        
        doc += " */\n"
        return doc
    
    # Regular expression to match function declarations
    # This pattern matches function declarations in both .hpp and .cpp files
    func_pattern = re.compile(r'^\s*(?:virtual\s+)?(?:static\s+)?(?:inline\s+)?(?:explicit\s+)?'
                             r'((?:const\s+)?(?:[\w:]+(?:<[^>]*>)?(?:\s*\*|\s*&|\s+))*)'  # return type
                             r'([\w~]+(?:<[^>]*>)?)'  # function name
                             r'\s*\((.*?)\)'  # parameters
                             r'(?:\s*const)?'  # const qualifier
                             r'(?:\s*noexcept)?'  # noexcept specifier
                             r'(?:\s*override)?'  # override specifier
                             r'(?:\s*=\s*0)?'  # pure virtual
                             r'(?:\s*\{|\s*;)', re.MULTILINE | re.DOTALL)
    
    # Regular expression to match existing documentation
    doc_pattern = re.compile(r'/\*\*.*?\*/\s*$|///.*$', re.MULTILINE | re.DOTALL)
    
    # Find all .hpp and .cpp files in the hydra_math directory
    for root, _, files in os.walk(directory):
        # Only process files in hydra_math directory
        if "hydra_math" not in root:
            continue
            
        for filename in files:
            if filename.lower().endswith(('.hpp', '.cpp')):
                filepath = os.path.join(root, filename)
                
                try:
                    # Read file content
                    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                    
                    # Split content into lines for processing
                    lines = content.split('\n')
                    modified_lines = lines.copy()
                    changes_made = False
                    
                    # Find function declarations
                    matches = list(func_pattern.finditer(content))
                    
                    for match in matches:
                        # Get function details
                        return_type = match.group(1).strip()
                        func_name = match.group(2).strip()
                        params_str = match.group(3).strip()
                        
                        # Skip if it's a constructor or destructor
                        if func_name.startswith('~') or func_name == filename.split('.')[0]:
                            continue
                        
                        # Parse parameters
                        params = []
                        if params_str:
                            # Handle complex parameter lists with templates, etc.
                            bracket_count = 0
                            current_param = ""
                            for char in params_str:
                                if char == ',' and bracket_count == 0:
                                    params.append(current_param.strip())
                                    current_param = ""
                                else:
                                    current_param += char
                                    if char == '<':
                                        bracket_count += 1
                                    elif char == '>':
                                        bracket_count -= 1
                            if current_param:
                                params.append(current_param.strip())
                        
                        # Get the start line of the function
                        func_start_line = content[:match.start()].count('\n')
                        
                        # Check if there's already documentation
                        has_doc = False
                        for i in range(max(0, func_start_line - 5), func_start_line):
                            if i < len(lines) and ('/**' in lines[i] or '///' in lines[i]):
                                has_doc = True
                                break
                        
                        # Add documentation if not already present
                        if not has_doc:
                            # Generate documentation
                            doc = generate_documentation(func_name, params, return_type)
                            
                            # Insert documentation before function
                            modified_lines.insert(func_start_line, doc.rstrip())
                            changes_made = True
                            print(f"In {filepath}:")
                            print(f"  Adding documentation for function: {func_name}")
                    
                    # Write changes to file if any were made
                    if changes_made:
                        modified_content = '\n'.join(modified_lines)
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
    
    print(f"Adding documentation to undocumented functions in {directory}")
    if dry_run:
        print("DRY RUN MODE: No files will be modified")
    print("=" * 80)
    
    updated_files, skipped_files = document_functions(directory, dry_run)
    
    print("\nSummary:")
    print(f"- Total files updated: {len(updated_files)}")
    print(f"- Total files skipped: {len(skipped_files)}")
    
    if updated_files:
        print("\nUpdated files:")
        for file in updated_files:
            print(f"  {file}")
    
    if skipped_files:
        print("\nSkipped files:")
        for file, reason in skipped_files:
            print(f"  {file} - Reason: {reason}")

if __name__ == "__main__":
    main()
