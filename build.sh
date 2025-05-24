#!/bin/bash

# Exit on error
set -e

# Default values
BUILD_TYPE=Release
CLEAN_BUILD=false
RUN_EXAMPLE=false

# Get number of CPU cores (macOS compatible)
if command -v nproc >/dev/null 2>&1; then
    # Linux
    NUM_JOBS=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
    # macOS
    NUM_JOBS=$(sysctl -n hw.ncpu)
else
    # Fallback
    NUM_JOBS=4
fi

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE=Debug
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --run-example)
            RUN_EXAMPLE=true
            shift
            ;;
        -j*)
            NUM_JOBS=${1#-j}
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "🔨 Building AirPhoto Viewer..."
echo "   Build type: $BUILD_TYPE"
echo "   Jobs: $NUM_JOBS"

# Create build directory if it doesn't exist
# Convert BUILD_TYPE to lowercase for directory name
BUILD_DIR="build-$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')"
mkdir -p "$BUILD_DIR"

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo "🧹 Cleaning build directory..."
    rm -rf "$BUILD_DIR"/*
fi

# Navigate to build directory
cd "$BUILD_DIR"

# Run CMake
echo "🛠️  Configuring project..."
cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_INSTALL_PREFIX=../install

# Build the project
echo "🚀 Building project..."
cmake --build . --config $BUILD_TYPE -- -j$NUM_JOBS

# Install the project
echo "📦 Installing project..."
cmake --install . --config $BUILD_TYPE

# Copy the executable to the project root for convenience
if [ -f "airphoto_viewer" ]; then
    cp airphoto_viewer ..
fi

echo -e "\n✅ Build completed successfully!"
echo -e "\nAvailable executables:"
echo -e "  ./airphoto_viewer - Main application"
echo -e "  ./measurement_example - Measurement tool example"

# Run the example if requested
if [ "$RUN_EXAMPLE" = true ]; then
    echo -e "🚀 Starting measurement example...\n"
    ./measurement_example
fi
