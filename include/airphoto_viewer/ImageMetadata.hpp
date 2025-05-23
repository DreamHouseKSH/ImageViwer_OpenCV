#pragma once

#include <string>
#include <map>
#include <memory>
#include <opencv2/core.hpp>

namespace airphoto {

/**
 * @brief 이미지 메타데이터를 저장하는 클래스
 */
class ImageMetadata {
public:
    ImageMetadata();
    ~ImageMetadata() = default;

    // 기본 이미지 정보
    int width = 0;
    int height = 0;
    int channels = 0;
    std::string colorSpace;
    std::string format;
    std::pair<double, double> dpi{0.0, 0.0};
    bool hasAlpha = false;

    // 카메라 정보
    std::string cameraMake;
    std::string cameraModel;
    std::string datetimeOriginal;
    double exposureTime = 0.0;
    double fNumber = 0.0;
    int isoSpeed = 0;
    double focalLength = 0.0;

    // GPS 정보
    double gpsLatitude = 0.0;
    double gpsLongitude = 0.0;
    double gpsAltitude = 0.0;

    // EXIF 데이터
    std::map<std::string, std::string> exifData;

    // 색상 프로파일
    struct ColorProfile {
        std::string profileType;
        size_t profileSize = 0;
        std::vector<uint8_t> profileData;
    };
    std::shared_ptr<ColorProfile> colorProfile;

    /**
     * @brief 메타데이터를 JSON 형식의 문자열로 변환합니다.
     * @return JSON 형식의 문자열
     */
    std::string toJson() const;

    /**
     * @brief 메타데이터를 사전 형태로 변환합니다.
     * @return 메타데이터를 담은 맵
     */
    std::map<std::string, std::string> toMap() const;
};

} // namespace airphoto
