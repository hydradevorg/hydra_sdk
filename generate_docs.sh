#!/bin/bash

# Check if GitDoc is installed
if ! command -v gitdoc &> /dev/null; then
    echo "GitDoc is not installed. Installing..."
    npm install -g gitdoc
fi

# Generate documentation
echo "Generating documentation..."
gitdoc generate

# Copy custom documentation
echo "Copying custom documentation..."
cp -r docs/custom/* docs/gitdoc/

# Create a symlink to the HTML documentation
echo "Creating symlink to HTML documentation..."
ln -sf gitdoc docs/html

echo "Documentation generated successfully!"
echo "You can view the documentation by opening docs/html/index.html in a web browser."
