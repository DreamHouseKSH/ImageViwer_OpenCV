#!/bin/bash

# Exit on error
set -e

echo "🔨 Building AirPhoto Viewer..."

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Run CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build the project
cmake --build . --config Release -- -j$(nproc)

echo "\n✅ Build completed successfully!"
echo "\nTo run the application:"
echo "  ./airphoto_viewer [image_file]"

# Copy the executable to the project root for convenience
cp airphoto_viewer ..
