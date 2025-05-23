# Minimum required CMake version
cmake_minimum_required(VERSION 3.16)

# Project name and version
project(MeasurementExample VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
find_package(OpenCV REQUIRED)

# Add the airphoto_viewer library
add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/airphoto_viewer)

# Add the example executable
add_executable(measurement_example
    measurement_example.cpp
)

# Link libraries
target_link_libraries(measurement_example PRIVATE
    airphoto_viewer
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    ${OpenCV_LIBS}
)

# Set target properties
target_compile_definitions(measurement_example PRIVATE
    $<$<CONFIG:Debug>:QT_QML_DEBUG>
)

target_compile_features(measurement_example PRIVATE cxx_std_17)

# Install target
install(TARGETS measurement_example
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Copy QML files to build directory
file(GLOB QML_FILES "*.qml")
file(COPY ${QML_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Create a run target for convenience
add_custom_target(run_measurement_example
    COMMAND $<TARGET_FILE:measurement_example>
    DEPENDS measurement_example
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
