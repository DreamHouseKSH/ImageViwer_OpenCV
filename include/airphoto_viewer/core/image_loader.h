#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <QString>
#include <QImage>
#include <opencv2/opencv.hpp>

namespace airphoto_viewer {
namespace core {

/**
 * @brief 이미지 로딩 및 처리를 담당하는 클래스
 * 
 * 이 클래스는 이미지 파일을 로드하고, 다양한 형식의 이미지를 처리하며,
 * OpenCV와 QImage 간의 변환을 담당합니다.
 */
class ImageLoader {
public:
    /**
     * @brief 이미지 로드 결과를 나타내는 구조체
     */
    struct LoadResult {
        bool success = false;      ///< 로딩 성공 여부
        QImage image;              ///< 로드된 QImage
        QString errorMessage;      ///< 오류 메시지 (실패 시)
        int width = 0;            ///< 이미지 너비 (픽셀)
        int height = 0;           ///< 이미지 높이 (픽셀)
        QString format;            ///< 이미지 형식 (예: "PNG", "JPEG")
    };

    /**
     * @brief 이미지 파일을 로드합니다.
     * @param filePath 로드할 이미지 파일 경로
     * @return LoadResult 이미지 로딩 결과
     */
    static LoadResult loadImage(const QString &filePath);

    /**
     * @brief OpenCV Mat을 QImage로 변환합니다.
     * @param mat 변환할 OpenCV Mat
     * @return 변환된 QImage
     */
    static QImage matToQImage(const cv::Mat &mat);

private:
    // QImageReader를 사용한 이미지 로드 시도
    static LoadResult loadWithQImageReader(const QString &filePath);
    
    // OpenCV를 사용한 이미지 로드 시도 (QImageReader 실패 시 대체용)
    static LoadResult loadWithOpenCV(const QString &filePath);
};

} // namespace core
} // namespace airphoto_viewer

#endif // IMAGE_LOADER_H
