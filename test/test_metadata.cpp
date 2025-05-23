#include <iostream>
#include <string>
#include <iomanip>
#include <chrono>
#include <fstream>

#include "airphoto_viewer/ImageData.hpp"

using namespace airphoto;
using namespace std::chrono;

std::string formatBytes(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024 && unit < 4) {
        size /= 1024;
        unit++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unit];
    return oss.str();
}

void printSection(const std::string& title, const std::map<std::string, std::string>& data, int indent = 2) {
    std::string indentStr(indent, ' ');
    
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << title << "\n";
    std::cout << std::string(50, '-') << "\n";
    
    if (data.empty()) {
        std::cout << indentStr << "No data available\n";
        return;
    }
    
    for (const auto& [key, value] : data) {
        std::cout << indentStr << key << ": " << value << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path> [--save]\n";
        return 1;
    }
    
    std::string imagePath = argv[1];
    bool saveMetadata = (argc >= 3 && std::string(argv[2]) == "--save");
    
    try {
        // Display test header
        std::cout << "\n" << std::string(50, '=') << "\n";
        std::cout << "IMAGE METADATA EXTRACTION TEST\n";
        std::cout << std::string(50, '=') << "\n";
        std::cout << "Image path: " << imagePath << "\n";
        
        // Get file info
        std::ifstream file(imagePath, std::ios::binary | std::ios::ate);
        if (!file) {
            throw std::runtime_error("Failed to open file: " + imagePath);
        }
        
        auto fileSize = file.tellg();
        file.close();
        
        std::cout << "File size: " << formatBytes(fileSize) << "\n";
        
        // Load image and measure time
        auto start = high_resolution_clock::now();
        
        std::cout << "\nLoading image and extracting metadata..." << std::endl;
        ImageData imageData(imagePath);
        
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);
        
        std::cout << "Loading and extraction completed in " << duration.count() << " ms\n";
        
        if (!imageData.isLoaded()) {
            throw std::runtime_error("Failed to load image");
        }
        
        // Get metadata
        auto metadata = imageData.getMetadata();
        auto metaMap = metadata->toMap();
        
        // Print basic metadata
        std::map<std::string, std::string> basicMeta = {
            {"Image Size", metaMap["width"] + " x " + metaMap["height"] + " pixels"},
            {"Color Channels", metaMap["channels"] + " (" + metaMap["colorSpace"] + ")"},
            {"Image Format", metaMap["format"]},
            {"Resolution", metaMap["dpi"]},
            {"Has Alpha Channel", metaMap["hasAlpha"]}
        };
        printSection("BASIC IMAGE INFORMATION", basicMeta);
        
        // Print camera information if available
        if (!metadata->cameraMake.empty() || !metadata->cameraModel.empty()) {
            std::map<std::string, std::string> cameraInfo = {
                {"Make", metadata->cameraMake.empty() ? "Unknown" : metadata->cameraMake},
                {"Model", metadata->cameraModel.empty() ? "Unknown" : metadata->cameraModel},
                {"Date/Time", metadata->datetimeOriginal.empty() ? "Unknown" : metadata->datetimeOriginal},
                {"Exposure Time", metadata->exposureTime > 0 ? std::to_string(metadata->exposureTime) + " sec" : "Unknown"},
                {"F-Number", metadata->fNumber > 0 ? "f/" + std::to_string(metadata->fNumber) : "Unknown"},
                {"ISO", metadata->isoSpeed > 0 ? std::to_string(metadata->isoSpeed) : "Unknown"},
                {"Focal Length", metadata->focalLength > 0 ? std::to_string(metadata->focalLength) + " mm" : "Unknown"}
            };
            printSection("CAMERA INFORMATION", cameraInfo);
        }
        
        // Print GPS information if available
        if (metadata->gpsLatitude != 0 || metadata->gpsLongitude != 0) {
            std::map<std::string, std::string> gpsInfo = {
                {"Latitude", std::to_string(std::abs(metadata->gpsLatitude)) + "° " + 
                               (metadata->gpsLatitude >= 0 ? "N" : "S")},
                {"Longitude", std::to_string(std::abs(metadata->gpsLongitude)) + "° " + 
                                (metadata->gpsLongitude >= 0 ? "E" : "W")},
                {"Altitude", metadata->gpsAltitude != 0 ? std::to_string(metadata->gpsAltitude) + " m" : "Unknown"}
            };
            printSection("LOCATION INFORMATION", gpsInfo);
        }
        
        // Print color profile information if available
        if (metadata->colorProfile) {
            std::map<std::string, std::string> colorProfile = {
                {"Type", metadata->colorProfile->profileType},
                {"Size", formatBytes(metadata->colorProfile->profileSize)}
            };
            printSection("COLOR PROFILE", colorProfile);
        }
        
        // Save metadata to JSON if requested
        if (saveMetadata) {
            std::string outputPath = imagePath.substr(0, imagePath.find_last_of('.')) + "_metadata.json";
            std::ofstream outFile(outputPath);
            if (outFile) {
                outFile << metadata->toJson();
                std::cout << "\n[SUCCESS] Metadata saved to: " << outputPath << "\n";
            } else {
                std::cerr << "\n[ERROR] Failed to save metadata to: " << outputPath << "\n";
            }
        }
        
        std::cout << "\n" << std::string(50, '=') << "\n";
        std::cout << "TEST COMPLETED SUCCESSFULLY\n";
        std::cout << std::string(50, '=') << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
