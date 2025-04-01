#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Define the build directory
BUILD_DIR="build"

# Check if the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
  echo "Error: Build directory '$BUILD_DIR' not found." >&2
  echo "Please run the build script first (./build.sh)." >&2
  exit 1
fi

# Store the current directory
ORIGINAL_DIR=$(pwd)

# Navigate into the build directory
echo "Changing to directory: $BUILD_DIR"
cd "$BUILD_DIR"

# Run the tests using CTest
echo "Running tests..."
ctest --output-on-failure

# Navigate back to the original directory
echo "Changing back to directory: $ORIGINAL_DIR"
cd "$ORIGINAL_DIR"

echo "Tests complete." 