#!/bin/bash

# Build script for 3cc compiler

set -e  # Exit on error

cd "$(dirname "$0")"

echo "Building 3cc compiler..."

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake ..
cmake --build .

echo
echo "Build complete! Compiler is at: build/3cc"
echo
echo "To run tests: cd test && ./test.sh"
