#pragma once

#include "airphoto_viewer/ImageMetadata.hpp"
#include <opencv2/core/mat.hpp>
#include <string>
#include <memory>

namespace airphoto {

/**
 * @brief 이미지 데이터와 메타데이터를 관리하는 클래스
 */
class ImageData {
public:
    /**
     * @brief 생성자
     * @param filepath 로드할 이미지 파일 경로
     */
    explicit ImageData(const std::string& filepath);
    
    ~ImageData() = default;

    /**
     * @brief 이미지가 성공적으로 로드되었는지 확인
     * @return 로드 성공 시 true, 실패 시 false
     */
    bool isLoaded() const;

    /**
     * @brief 이미지 데이터를 가져옵니다.
     * @return OpenCV Mat 객체의 shared_ptr
     */
    std::shared_ptr<cv::Mat> getImageData() const;

    /**
     * @brief 이미지 메타데이터를 가져옵니다.
     * @return ImageMetadata 객체의 shared_ptr
     */
    std::shared_ptr<ImageMetadata> getMetadata() const;

    /**
     * @brief 이미지를 지정된 경로에 저장합니다.
     * @param filepath 저장할 파일 경로
     * @return 저장 성공 시 true, 실패 시 false
     */
    bool save(const std::string& filepath) const;

private:
    std::string filepath_;
    std::shared_ptr<cv::Mat> imageData_;
    std::shared_ptr<ImageMetadata> metadata_;

    /**
     * @brief 이미지 파일에서 메타데이터를 추출합니다.
     */
    void extractMetadata();

    /**
     * @brief EXIF 데이터에서 특정 태그의 값을 가져옵니다.
     * @param exifData EXIF 데이터 맵
     * @param tag 찾을 태그
     * @return 태그에 해당하는 값이 있으면 해당 값, 없으면 빈 문자열
     */
    std::string getExifValue(const std::map<std::string, std::string>& exifData, const std::string& tag) const;

    /**
     * @brief GPS 좌표를 도(degree) 형식으로 변환합니다.
     * @param coord 도/분/초 형식의 좌표
     * @param ref 방향 참조 (N/S/E/W)
     * @return 도(degree) 형식의 좌표
     */
    double convertGpsCoordinate(const std::string& coord, const std::string& ref) const;
};

} // namespace airphoto
