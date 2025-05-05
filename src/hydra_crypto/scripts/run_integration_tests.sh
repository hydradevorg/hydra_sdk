#!/bin/bash
# Integration test script for hydra_crypto library

set -e  # Exit on error

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
BUILD_DIR="${PROJECT_ROOT}/build"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Running hydra_crypto integration tests${NC}"

# Ensure build directory exists
if [ ! -d "${BUILD_DIR}" ]; then
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir -p "${BUILD_DIR}"
fi

# Navigate to build directory
cd "${BUILD_DIR}"

# Configure with tests enabled
echo -e "${YELLOW}Configuring project with tests enabled...${NC}"
cmake -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON ..

# Build the project
echo -e "${YELLOW}Building project...${NC}"
cmake --build . -- -j$(nproc)

# Run the tests
echo -e "${YELLOW}Running tests...${NC}"
ctest --output-on-failure

# Run the examples as additional tests
echo -e "${YELLOW}Running examples...${NC}"
make run_examples

echo -e "${GREEN}All tests passed successfully!${NC}"
