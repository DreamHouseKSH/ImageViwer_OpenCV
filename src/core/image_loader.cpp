#include "airphoto_viewer/core/image_loader.h"
#include <QImageReader>
#include <QFileInfo>
#include <QDebug>

namespace airphoto_viewer {
namespace core {

ImageLoader::LoadResult ImageLoader::loadImage(const QString &filePath) {
    qDebug() << "[ImageLoader] Loading image:" << filePath;
    
    // 1. 파일 존재 및 읽기 권한 확인
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "[ImageLoader] File does not exist:" << filePath;
        return {false, {}, "파일이 존재하지 않습니다: " + filePath};
    }
    
    if (!fileInfo.isReadable()) {
        qWarning() << "[ImageLoader] File is not readable:" << filePath;
        return {false, {}, "파일에 접근할 수 없습니다 (권한 확인 필요): " + filePath};
    }
    
    // 2. QImageReader로 먼저 시도 (더 나은 성능 및 메타데이터 지원)
    auto result = loadWithQImageReader(filePath);
    if (result.success) {
        return result;
    }
    
    // 3. QImageReader 실패 시 OpenCV로 재시도
    qDebug() << "[ImageLoader] QImageReader failed, trying OpenCV...";
    return loadWithOpenCV(filePath);
}

ImageLoader::LoadResult ImageLoader::loadWithQImageReader(const QString &filePath) {
    LoadResult result;
    QImageReader reader(filePath);
    reader.setAutoTransform(true);
    
    // 이미지 형식 및 크기 정보 저장
    result.format = QString(reader.format()).toUpper();
    
    // 이미지 로드 시도
    result.image = reader.read();
    if (result.image.isNull()) {
        result.errorMessage = QString("이미지 로드 실패: %1").arg(reader.errorString());
        qWarning() << "[ImageLoader] QImageReader error:" << result.errorMessage;
        return result;
    }
    
    // 성공 시 결과 설정
    result.success = true;
    result.width = result.image.width();
    result.height = result.image.height();
    
    qDebug() << "[ImageLoader] Successfully loaded with QImageReader:"
             << result.width << "x" << result.height << "format:" << result.format;
    
    return result;
}

ImageLoader::LoadResult ImageLoader::loadWithOpenCV(const QString &filePath) {
    LoadResult result;
    
    try {
        // OpenCV로 이미지 로드 (BGR 포맷)
        cv::Mat cvImage = cv::imread(filePath.toStdString(), cv::IMREAD_COLOR);
        
        if (cvImage.empty()) {
            result.errorMessage = "OpenCV로 이미지를 로드할 수 없습니다";
            qWarning() << "[ImageLoader] OpenCV failed to load image:" << filePath;
            return result;
        }
        
        // OpenCV Mat을 QImage로 변환
        result.image = matToQImage(cvImage);
        
        // 결과 설정
        result.success = true;
        result.width = result.image.width();
        result.height = result.image.height();
        result.format = QFileInfo(filePath).suffix().toUpper();
        
        qDebug() << "[ImageLoader] Successfully loaded with OpenCV:"
                 << result.width << "x" << result.height;
        
    } catch (const cv::Exception &e) {
        result.errorMessage = QString("OpenCV 오류: %1").arg(e.what());
        qCritical() << "[ImageLoader] OpenCV exception:" << e.what();
    } catch (const std::exception &e) {
        result.errorMessage = QString("오류 발생: %1").arg(e.what());
        qCritical() << "[ImageLoader] Exception:" << e.what();
    }
    
    return result;
}

QImage ImageLoader::matToQImage(const cv::Mat &mat) {
    // OpenCV Mat을 QImage로 변환
    switch (mat.type()) {
        case CV_8UC4: {  // 8-bit 4 channel
            return QImage(mat.data, mat.cols, mat.rows, 
                         static_cast<int>(mat.step), 
                         QImage::Format_ARGB32).copy();
        }
        case CV_8UC3: {  // 8-bit 3 channel
            cv::Mat rgb;
            cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
            return QImage(rgb.data, rgb.cols, rgb.rows, 
                         static_cast<int>(rgb.step), 
                         QImage::Format_RGB888).copy();
        }
        case CV_8UC1: {  // 8-bit 1 channel
            return QImage(mat.data, mat.cols, mat.rows, 
                         static_cast<int>(mat.step),
                         QImage::Format_Grayscale8).copy();
        }
        default: {
            qWarning() << "[ImageLoader] Unsupported Mat format:" << mat.type();
            return QImage();
        }
    }
}

} // namespace core
} // namespace airphoto_viewer
