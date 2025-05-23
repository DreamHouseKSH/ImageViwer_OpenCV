#include "airphoto_viewer/core/tile.h"
#include <QPainter>

namespace airphoto_viewer {
namespace core {

Tile::Tile(int level, int x, int y, int tileSize)
    : m_level(level)
    , m_x(x)
    , m_y(y)
    , m_tileSize(tileSize)
{
}

Tile::~Tile() {
    // Clean up resources if needed
}

void Tile::loadFromMat(const cv::Mat& source, const cv::Rect& sourceRect) {
    // Ensure the source rectangle is within bounds
    cv::Rect imageRect(0, 0, source.cols, source.rows);
    cv::Rect validRect = sourceRect & imageRect;
    
    if (validRect.area() == 0) {
        // No intersection, create empty tile
        imageData = cv::Mat::zeros(m_tileSize, m_tileSize, source.type());
        return;
    }
    
    // Extract the region of interest
    cv::Mat roi(source, validRect);
    
    // Resize to tile size if needed
    if (validRect.width != m_tileSize || validRect.height != m_tileSize) {
        cv::resize(roi, imageData, cv::Size(m_tileSize, m_tileSize), 0, 0, cv::INTER_AREA);
    } else {
        imageData = roi.clone();
    }
}

QImage Tile::toQImage() const {
    if (imageData.empty()) {
        return QImage();
    }
    
    // Convert OpenCV Mat to QImage
    switch (imageData.type()) {
        case CV_8UC1: {
            QImage img(imageData.data, imageData.cols, imageData.rows, 
                     static_cast<int>(imageData.step), QImage::Format_Grayscale8);
            return img.copy();
        }
        case CV_8UC3: {
            QImage img(imageData.data, imageData.cols, imageData.rows, 
                     static_cast<int>(imageData.step), QImage::Format_BGR888);
            return img.rgbSwapped();
        }
        case CV_8UC4: {
            QImage img(imageData.data, imageData.cols, imageData.rows, 
                     static_cast<int>(imageData.step), QImage::Format_ARGB32);
            return img.copy();
        }
        default:
            return QImage();
    }
}

QRect Tile::bounds(double scale) const {
    int scaledSize = static_cast<int>(m_tileSize * scale);
    int x = m_x * scaledSize;
    int y = m_y * scaledSize;
    return QRect(x, y, scaledSize, scaledSize);
}

} // namespace core
} // namespace airphoto_viewer
