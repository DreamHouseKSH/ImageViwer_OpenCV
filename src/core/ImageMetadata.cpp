#include "airphoto_viewer/ImageMetadata.hpp"
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace airphoto {

ImageMetadata::ImageMetadata() = default;

std::string ImageMetadata::toJson() const {
    json j;
    
    // 기본 이미지 정보
    j["width"] = width;
    j["height"] = height;
    j["channels"] = channels;
    j["colorSpace"] = colorSpace;
    j["format"] = format;
    j["dpi"] = {dpi.first, dpi.second};
    j["hasAlpha"] = hasAlpha;
    
    // 카메라 정보
    j["cameraMake"] = cameraMake;
    j["cameraModel"] = cameraModel;
    j["datetimeOriginal"] = datetimeOriginal;
    j["exposureTime"] = exposureTime;
    j["fNumber"] = fNumber;
    j["isoSpeed"] = isoSpeed;
    j["focalLength"] = focalLength;
    
    // GPS 정보
    j["gpsLatitude"] = gpsLatitude;
    j["gpsLongitude"] = gpsLongitude;
    j["gpsAltitude"] = gpsAltitude;
    
    // EXIF 데이터
    json exifJson;
    for (const auto& [key, value] : exifData) {
        exifJson[key] = value;
    }
    j["exifData"] = exifJson;
    
    // 색상 프로파일
    if (colorProfile) {
        j["colorProfile"] = {
            {"profileType", colorProfile->profileType},
            {"profileSize", colorProfile->profileSize}
        };
    }
    
    return j.dump(2); // 들여쓰기 2칸으로 포맷팅
}

std::map<std::string, std::string> ImageMetadata::toMap() const {
    std::map<std::string, std::string> result;
    
    // 기본 이미지 정보
    result["width"] = std::to_string(width);
    result["height"] = std::to_string(height);
    result["channels"] = std::to_string(channels);
    result["colorSpace"] = colorSpace;
    result["format"] = format;
    result["dpi"] = std::to_string(dpi.first) + " x " + std::to_string(dpi.second);
    result["hasAlpha"] = hasAlpha ? "true" : "false";
    
    // 카메라 정보
    result["cameraMake"] = cameraMake;
    result["cameraModel"] = cameraModel;
    result["datetimeOriginal"] = datetimeOriginal;
    result["exposureTime"] = std::to_string(exposureTime);
    result["fNumber"] = std::to_string(fNumber);
    result["isoSpeed"] = std::to_string(isoSpeed);
    result["focalLength"] = std::to_string(focalLength);
    
    // GPS 정보
    result["gpsLatitude"] = std::to_string(gpsLatitude);
    result["gpsLongitude"] = std::to_string(gpsLongitude);
    result["gpsAltitude"] = std::to_string(gpsAltitude);
    
    return result;
}

} // namespace airphoto
