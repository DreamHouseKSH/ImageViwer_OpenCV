#include <iostream>
#include <string>
#include "airphoto_viewer/ImageData.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <image_path>\n";
        return 1;
    }

    std::string imagePath = argv[1];
    
    try {
        std::cout << "Loading image: " << imagePath << std::endl;
        airphoto::ImageData imageData(imagePath);
        
        auto metadata = imageData.getMetadata();
        
        std::cout << "\n=== Image Metadata ===" << std::endl;
        std::cout << "Size: " << metadata->width << " x " << metadata->height << std::endl;
        std::cout << "Channels: " << metadata->channels << std::endl;
        std::cout << "Format: " << metadata->format << std::endl;
        
        if (!metadata->cameraMake.empty() || !metadata->cameraModel.empty()) {
            std::cout << "\n=== Camera Information ===" << std::endl;
            if (!metadata->cameraMake.empty()) std::cout << "Make: " << metadata->cameraMake << std::endl;
            if (!metadata->cameraModel.empty()) std::cout << "Model: " << metadata->cameraModel << std::endl;
            if (!metadata->datetimeOriginal.empty()) std::cout << "Date/Time: " << metadata->datetimeOriginal << std::endl;
        }
        
        if (metadata->gpsLatitude != 0 || metadata->gpsLongitude != 0) {
            std::cout << "\n=== GPS Information ===" << std::endl;
            std::cout << "Latitude: " << metadata->gpsLatitude << std::endl;
            std::cout << "Longitude: " << metadata->gpsLongitude << std::endl;
            if (metadata->gpsAltitude != 0) std::cout << "Altitude: " << metadata->gpsAltitude << " m" << std::endl;
        }
        
        if (!metadata->exifData.empty()) {
            std::cout << "\n=== EXIF Data (First 5 items) ===" << std::endl;
            int count = 0;
            for (const auto& [key, value] : metadata->exifData) {
                std::cout << key << ": " << value << std::endl;
                if (++count >= 5) break;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
