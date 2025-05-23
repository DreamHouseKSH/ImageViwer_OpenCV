#ifndef AIRPHOTO_VIEWER_TILE_VIEW_H
#define AIRPHOTO_VIEWER_TILE_VIEW_H

#include <QWidget>
#include <QImage>
#include <QPoint>
#include <QTimer>
#include <memory>
#include "../core/tile_cache.h"

namespace airphoto_viewer {
namespace ui {

/**
 * @brief A widget that displays a large image using a tile-based approach
 * 
 * This widget handles:
 * - Panning with mouse drag
 * - Zooming with mouse wheel or touchpad
 * - Loading and displaying tiles asynchronously
 * - Smooth animations for pan/zoom
 */
class TileView : public QWidget {
    Q_OBJECT
    
public:
    explicit TileView(QWidget* parent = nullptr);
    ~TileView() override;
    
    /**
     * @brief Load an image file
     * 
     * @param filePath Path to the image file
     * @return true if the image was loaded successfully
     */
    bool loadImage(const QString& filePath);
    
    /**
     * @brief Fit the image to the current view
     */
    void fitToView();
    
    /**
     * @brief Reset the view to show the entire image at 1:1 scale
     */
    void resetView();
    
    /**
     * @brief Zoom in/out by a factor
     * 
     * @param factor Zoom factor (e.g., 1.25 for zoom in, 0.8 for zoom out)
     * @param mousePos Position to zoom towards (in widget coordinates)
     */
    void zoom(double factor, const QPoint& mousePos = QPoint());
    
    /**
     * @brief Get the current scale factor
     * 
     * @return double The current scale factor (1.0 = 100%)
     */
    double scaleFactor() const { return m_scale; }
    
    /**
     * @brief Get the current offset
     * 
     * @return QPointF The current offset in image coordinates
     */
    QPointF offset() const { return m_offset; }
    
    /**
     * @brief Get the source image size
     * 
     * @return QSize The size of the source image in pixels
     */
    QSize sourceSize() const;
    
    /**
     * @brief Get the visible area in image coordinates
     * 
     * @return QRectF The visible rectangle in image coordinates
     */
    QRectF visibleImageRect() const;
    
    /**
     * @brief Convert from widget coordinates to image coordinates
     * 
     * @param pos Position in widget coordinates
     * @return QPointF Position in image coordinates
     */
    QPointF mapToImage(const QPoint& pos) const;
    
    /**
     * @brief Convert from image coordinates to widget coordinates
     * 
     * @param pos Position in image coordinates
     * @return QPoint Position in widget coordinates
     */
    QPoint mapFromImage(const QPointF& pos) const;
    
    // QWidget overrides
    QSize sizeHint() const override;
    
signals:
    /**
     * @brief Emitted when the view transformation changes
     */
    void viewChanged();
    
    /**
     * @brief Emitted when the image is loaded
     */
    void imageLoaded();
    
protected:
    // Event handlers
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    bool event(QEvent* event) override;
    
private slots:
    void updateTiles();
    void animationTick();
    
private:
    // Animation state
    struct Animation {
        QPointF startOffset;
        QPointF targetOffset;
        double startScale;
        double targetScale;
        qint64 startTime;
        qint64 duration;  // in ms
        bool active;
    };
    
    // View state
    double m_scale;
    QPointF m_offset;
    QPoint m_lastMousePos;
    bool m_panning;
    
    // Animation
    Animation m_animation;
    QTimer m_animationTimer;
    
    // Tiles
    std::unique_ptr<core::TileCache> m_tileCache;
    
    // Performance
    QTimer m_updateTimer;
    bool m_updatePending;
    QRect m_visibleTileArea;
    
    // Private methods
    void updateTileGrid();
    void startAnimation(const QPointF& targetOffset, double targetScale, int duration = 200);
    void stopAnimation();
    void ensureVisible(const QPointF& imgPos);
    QRectF imageRect() const;
    QRectF viewportRect() const;
    void scheduleUpdate();
};

} // namespace ui
} // namespace airphoto_viewer

#endif // AIRPHOTO_VIEWER_TILE_VIEW_H
