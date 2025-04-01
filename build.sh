#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Create build directory if it doesn't exist
BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
  echo "Creating build directory: $BUILD_DIR"
  mkdir "$BUILD_DIR"
fi

# Navigate into the build directory
cd "$BUILD_DIR"

# Configure the project with CMake
# Add -DCMAKE_PREFIX_PATH=/path/to/eigen if Eigen is not in a standard location
echo "Configuring CMake..."
cmake ..

# Build the project
echo "Building project..."
cmake --build .

echo "Build complete. Executables are in the build/ directory."
echo "Run tests with: ctest (from the build directory)" 