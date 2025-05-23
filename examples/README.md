# Measurement Tool Examples

This directory contains example applications that demonstrate how to use the Measurement Tool in different scenarios.

## Examples

### 1. QML Example (`MeasurementExample.qml`)

A pure QML example that shows how to use the `MeasurementTool` and `MeasurementOverlay` components in a QML application.

**Features:**
- Distance measurement between two or more points
- Area measurement for polygons
- Interactive UI with tool selection
- Real-time measurement display

**How to run:**

```bash
# Make sure QML modules are in the QML2_IMPORT_PATH
export QML2_IMPORT_PATH=$PWD/../qml:$QML2_IMPORT_PATH

# Run the example using qml
git clone https://github.com/yourusername/airphoto-viewer.git
cd airphoto-viewer/examples
qml MeasurementExample.qml
```

### 2. C++ with QML Example (`measurement_example.cpp`)

A C++ application that demonstrates how to integrate the measurement tool in a C++/QML application.

**Features:**
- Integration of C++ and QML
- Custom measurement tool registration
- Example of handling measurement data in C++

**How to build and run:**

```bash
# Clone the repository
git clone https://github.com/yourusername/airphoto-viewer.git
cd airphoto-viewer

# Create build directory and build
mkdir -p build && cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build . --target measurement_example

# Run the example
./examples/measurement_example
```

## Building the Examples

### Prerequisites

- Qt 6.0 or later
- CMake 3.16 or later
- C++17 compatible compiler
- OpenCV 4.5.0 or later

### Build Options

- `BUILD_EXAMPLES`: Build the example applications (ON/OFF, default: OFF)
- `BUILD_EXAMPLE_QML`: Build the QML example (ON/OFF, default: ON)
- `BUILD_EXAMPLE_CPP`: Build the C++ example (ON/OFF, default: ON)

Example:

```bash
cmake -DBUILD_EXAMPLES=ON -DBUILD_EXAMPLE_QML=ON -DBUILD_EXAMPLE_CPP=ON ..
cmake --build . --target all
```

## Usage Instructions

1. **Distance Measurement**
   - Click the "Distance Measurement" button
   - Click on the image to set the start point
   - Click again to set the end point
   - The distance will be displayed in meters

2. **Area Measurement**
   - Click the "Area Measurement" button
   - Click on the image to set the polygon vertices
   - Double-click to complete the polygon
   - The area will be displayed in square meters

3. **Clearing Measurements**
   - Click the "Clear" button or press the Delete key to remove the last point
   - Press Esc to clear all measurements

## Customization

You can customize the appearance of the measurement tool by setting the following properties:

```qml
MeasurementOverlay {
    tool: measurementTool
    lineColor: "red"      // Color of the measurement lines
    lineWidth: 2          // Width of the measurement lines
    textColor: "white"    // Color of the measurement text
    textSize: 12          // Size of the measurement text
    showHoverFeedback: true  // Show hover feedback (preview line)
}
```

## Troubleshooting

### QML Module Not Found
If you see an error like "module "AirPhoto.Viewer.Measurement" is not installed", make sure the QML modules are in the QML2_IMPORT_PATH:

```bash
export QML2_IMPORT_PATH=/path/to/airphoto-viewer/qml:$QML2_IMPORT_PATH
```

### Missing Dependencies
Make sure all required dependencies are installed:

- On Ubuntu/Debian:
  ```bash
  sudo apt-get install qt6-base-dev qt6-declarative-dev libopencv-dev
  ```

- On macOS (using Homebrew):
  ```bash
  brew install qt@6 opencv
  ```

- On Windows (using vcpkg):
  ```bash
  vcpkg install qt6-base:x64-windows opencv:x64-windows
  ```

## License

These examples are part of the AirPhoto Viewer project and are licensed under the MIT License.
