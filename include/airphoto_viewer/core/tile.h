#ifndef AIRPHOTO_VIEWER_TILE_H
#define AIRPHOTO_VIEWER_TILE_H

#include <opencv2/core.hpp>
#include <QRect>
#include <QImage>
#include <memory>

namespace airphoto_viewer {
namespace core {

/**
 * @brief Represents a single tile in the tile-based rendering system.
 * 
 * This class manages a portion of a larger image, allowing for efficient
 * loading and rendering of large images by only processing visible tiles.
 */
class Tile {
public:
    /**
     * @brief Construct a new Tile object
     * 
     * @param level The level of detail (0 = full resolution)
     * @param x The x-coordinate of the tile in the tile grid
     * @param y The y-coordinate of the tile in the tile grid
     * @param tileSize The size of the tile in pixels (assumed square)
     */
    Tile(int level, int x, int y, int tileSize = 256);
    
    ~Tile();

    /**
     * @brief Load the tile data from an OpenCV Mat
     * 
     * @param source The source image to extract the tile from
     * @param sourceRect The region in the source image to extract
     */
    void loadFromMat(const cv::Mat& source, const cv::Rect& sourceRect);
    
    /**
     * @brief Get the tile as a QImage for rendering
     * 
     * @return QImage The tile image data
     */
    QImage toQImage() const;
    
    /**
     * @brief Check if the tile has been loaded
     * 
     * @return true if the tile has image data
     * @return false otherwise
     */
    bool isLoaded() const { return !imageData.empty(); }
    
    /**
     * @brief Get the tile's level of detail
     * 
     * @return int The level (0 = full resolution, higher = more zoomed out)
     */
    int level() const { return m_level; }
    
    /**
     * @brief Get the tile's x-coordinate in the tile grid
     * 
     * @return int The x-coordinate
     */
    int x() const { return m_x; }
    
    /**
     * @brief Get the tile's y-coordinate in the tile grid
     * 
     * @return int The y-coordinate
     */
    int y() const { return m_y; }
    
    /**
     * @brief Get the size of the tile
     * 
     * @return int The tile size in pixels (assumed square)
     */
    int size() const { return m_tileSize; }
    
    /**
     * @brief Get the bounds of this tile in the source image
     * 
     * @param scale The current scale factor
     * @return QRect The bounds of the tile in the source image
     */
    QRect bounds(double scale = 1.0) const;

private:
    int m_level;     ///< Level of detail (0 = full resolution)
    int m_x;         ///< X coordinate in the tile grid
    int m_y;         ///< Y coordinate in the tile grid
    int m_tileSize;  ///< Size of the tile in pixels
    
    cv::Mat imageData;  ///< The actual image data for this tile
};

using TilePtr = std::shared_ptr<Tile>;

} // namespace core
} // namespace airphoto_viewer

#endif // AIRPHOTO_VIEWER_TILE_H
