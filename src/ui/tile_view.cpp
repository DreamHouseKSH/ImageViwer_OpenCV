#include "airphoto_viewer/ui/tile_view.h"
#include "airphoto_viewer/core/tile.h"
#include <QPainter>
#include <QApplication>
#include <QWheelEvent>
#include <QDebug>
#include <QtMath>
#include <QElapsedTimer>
#include <QThread>
#include <QDateTime>

namespace airphoto_viewer {
namespace ui {

TileView::TileView(QWidget* parent)
    : QWidget(parent)
    , m_scale(1.0)
    , m_offset(0, 0)
    , m_panning(false)
    , m_updatePending(false)
    , m_visibleTileArea(0, 0, 1, 1)
    , m_tileCache(std::make_unique<core::TileCache>(512))  // 512MB cache
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
    
    // Set up animation timer
    m_animationTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_animationTimer, &QTimer::timeout, this, &TileView::animationTick);
    
    // Set up update timer (for throttling tile updates)
    m_updateTimer.setSingleShot(true);
    m_updateTimer.setInterval(16);  // ~60 FPS
    connect(&m_updateTimer, &QTimer::timeout, this, &TileView::updateTiles);
    
    // Initialize animation state
    m_animation.active = false;
}

TileView::~TileView() = default;

bool TileView::loadImage(const QString& filePath) {
    qDebug() << "[TileView] Starting to load image:" << filePath;
    
    // Reset any previous state
    stopAnimation();
    
    // Clear any existing image
    m_tileCache->clear();
    
    // Update the view to show loading state
    update();
    QApplication::processEvents();
    
    qDebug() << "[TileView] Attempting to load image using TileCache...";
    
    // Try to load the new image
    bool success = m_tileCache->setSourceImage(filePath.toStdString());
    
    if (!success) {
        qCritical() << "[TileView] Failed to load image using TileCache:" << filePath;
        
        // Try loading with QImage to verify the file can be read
        QImage testImage;
        if (testImage.load(filePath)) {
            qDebug() << "[TileView] Image loaded successfully with QImage, size:" 
                     << testImage.size() << "format:" << testImage.format();
        } else {
            qCritical() << "[TileView] QImage also failed to load the image";
        }
        
        // Clear the cache again to ensure we're in a clean state
        m_tileCache->clear();
        update();
        return false;
    }
    
    qDebug() << "[TileView] Image loaded successfully, resetting view...";
    
    // Reset view to fit the new image
    resetView();
    
    // Update the display
    updateTileGrid();
    update();
    
    // Notify that the image has been loaded
    emit imageLoaded(filePath);
    
    qDebug() << "[TileView] Image loading completed successfully";
    return true;
}

void TileView::fitToView() {
    QSize imgSize = sourceSize();
    if (imgSize.isEmpty()) {
        return;
    }
    
    QSize viewSize = viewportRect().size().toSize();
    if (viewSize.isEmpty()) {
        return;
    }
    
    // Calculate scale to fit the image in the view
    double scaleX = static_cast<double>(viewSize.width()) / imgSize.width();
    double scaleY = static_cast<double>(viewSize.height()) / imgSize.height();
    double newScale = qMin(scaleX, scaleY);
    
    // Center the image
    QPointF newOffset(
        (viewSize.width() - imgSize.width() * newScale) / 2.0,
        (viewSize.height() - imgSize.height() * newScale) / 2.0
    );
    
    startAnimation(newOffset, newScale, 200);
}

void TileView::resetView() {
    QSize imgSize = sourceSize();
    if (imgSize.isEmpty()) {
        return;
    }
    
    // Center the image at 1:1 scale
    QSize viewSize = viewportRect().size().toSize();
    QPointF newOffset(
        (viewSize.width() - imgSize.width()) / 2.0,
        (viewSize.height() - imgSize.height()) / 2.0
    );
    
    startAnimation(newOffset, 1.0, 200);
}

void TileView::zoom(double factor, const QPoint& mousePos) {
    if (factor <= 0.0) {
        return;
    }
    
    QPointF imgPos = mapToImage(mousePos);
    double newScale = qBound(0.1, m_scale * factor, 100.0);
    
    // If we're at the min/max scale, don't zoom
    if (qFuzzyCompare(newScale, m_scale)) {
        return;
    }
    
    // Calculate the new offset to zoom towards the mouse position
    QPointF viewPos = mapFromImage(imgPos);
    QPointF newOffset = m_offset + (viewPos - mousePos) * (1.0 - 1.0 / factor);
    
    startAnimation(newOffset, newScale, 100);
}

QSize TileView::sourceSize() const {
    return m_tileCache->sourceSize();
}

QRectF TileView::visibleImageRect() const {
    QRectF viewRect = viewportRect();
    QPointF topLeft = mapToImage(viewRect.topLeft().toPoint());
    QPointF bottomRight = mapToImage(viewRect.bottomRight().toPoint());
    return QRectF(topLeft, bottomRight).normalized();
}

QPointF TileView::mapToImage(const QPoint& pos) const {
    return QPointF(
        (pos.x() - m_offset.x()) / m_scale,
        (pos.y() - m_offset.y()) / m_scale
    );
}

QPoint TileView::mapFromImage(const QPointF& pos) const {
    return QPoint(
        qRound(pos.x() * m_scale + m_offset.x()),
        qRound(pos.y() * m_scale + m_offset.y())
    );
}

QSize TileView::sizeHint() const {
    return QSize(800, 600);
}

void TileView::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillRect(rect(), Qt::darkGray);
    
    QSize imgSize = sourceSize();
    if (imgSize.isEmpty()) {
        return;
    }
    
    // Draw checkerboard background for transparent images
    QBrush checkerBrush(Qt::lightGray);
    painter.setPen(Qt::NoPen);
    
    const int checkerSize = 16;
    for (int y = 0; y < height(); y += checkerSize) {
        for (int x = ((y / checkerSize) % 2) * checkerSize; x < width(); x += checkerSize * 2) {
            painter.fillRect(x, y, checkerSize, checkerSize, checkerBrush);
        }
    }
    
    // Set up the transformation
    painter.save();
    painter.translate(m_offset);
    painter.scale(m_scale, m_scale);
    
    // Get the visible area in image coordinates
    QRectF visibleRect = visibleImageRect();
    
    // Calculate which tiles we need to draw
    int tileSize = 256;  // Should match the tile size in TileCache
    int startX = qFloor(visibleRect.left() / tileSize);
    int startY = qFloor(visibleRect.top() / tileSize);
    int endX = qCeil(visibleRect.right() / tileSize);
    int endY = qCeil(visibleRect.bottom() / tileSize);
    
    // Draw the tiles
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            // Get the tile (this will start loading it if needed)
            auto tile = m_tileCache->getTile(0, x, y);
            if (!tile) {
                continue;
            }
            
            // Calculate the position and size of this tile
            QRectF tileRect(
                x * tileSize,
                y * tileSize,
                tileSize,
                tileSize
            );
            
            // Only draw if the tile is in the visible area
            if (!tileRect.intersects(visibleRect)) {
                continue;
            }
            
            // Draw the tile
            if (tile->isLoaded()) {
                QImage tileImage = tile->toQImage();
                if (!tileImage.isNull()) {
                    painter.drawImage(tileRect, tileImage);
                }
            } else {
                // Draw a placeholder while loading
                painter.fillRect(tileRect, QColor(50, 50, 50, 128));
                painter.setPen(Qt::white);
                painter.drawText(tileRect, Qt::AlignCenter, "Loading...");
            }
        }
    }
    
    // Draw a border around the image
    painter.restore();
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(imageRect());
    
    // Draw a border around the viewport
    painter.setPen(QPen(Qt::red, 1, Qt::DotLine));
    painter.drawRect(viewportRect().adjusted(0, 0, -1, -1));
}

void TileView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    
    // Keep the image centered when the widget is resized
    if (!sourceSize().isEmpty()) {
        QPointF center = mapToImage(rect().center());
        QSizeF viewSize = viewportRect().size();
        QSizeF imgSize = sourceSize();
        
        m_offset = QPointF(
            (viewSize.width() - imgSize.width() * m_scale) / 2.0,
            (viewSize.height() - imgSize.height() * m_scale) / 2.0
        );
        
        // Adjust offset to keep the center point the same
        QPointF newCenter = mapFromImage(center);
        QPointF delta = rect().center() - newCenter;
        m_offset += delta;
    }
    
    updateTileGrid();
    update();
}

void TileView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_panning = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    
    QWidget::mousePressEvent(event);
}

void TileView::mouseMoveEvent(QMouseEvent* event) {
    if (m_panning) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_offset += delta;
        m_lastMousePos = event->pos();
        
        // Stop any ongoing animation
        stopAnimation();
        
        updateTileGrid();
        update();
        event->accept();
        return;
    }
    
    QWidget::mouseMoveEvent(event);
}

void TileView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_panning) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    
    QWidget::mouseReleaseEvent(event);
}

void TileView::wheelEvent(QWheelEvent* event) {
    // Use the delta to determine zoom direction
    double angle = event->angleDelta().y();
    double factor = qPow(1.2, angle / 120.0);
    
    zoom(factor, event->position().toPoint());
    event->accept();
}

void TileView::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Plus:
            zoom(1.25);
            break;
            
        case Qt::Key_Minus:
            zoom(0.8);
            break;
            
        case Qt::Key_0:
            resetView();
            break;
            
        case Qt::Key_F:
            fitToView();
            break;
            
        default:
            QWidget::keyPressEvent(event);
            return;
    }
    
    event->accept();
}

bool TileView::event(QEvent* event) {
    // Handle touch events for pinch-to-zoom
    if (event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchUpdate || event->type() == QEvent::TouchEnd) {
        QTouchEvent* touchEvent = static_cast<QTouchEvent*>(event);
        const auto touchPoints = touchEvent->points();
        
        if (touchPoints.count() == 2) {
            // Handle pinch gesture
            const auto& point0 = touchPoints[0];
            const auto& point1 = touchPoints[1];
            
            // Calculate the current and previous distances between the two touch points
            QPointF currentPos1 = point0.position();
            QPointF currentPos2 = point1.position();
            QPointF lastPos1 = point0.lastPosition();
            QPointF lastPos2 = point1.lastPosition();
            
            qreal currentDist = QLineF(currentPos1, currentPos2).length();
            qreal lastDist = QLineF(lastPos1, lastPos2).length();
            
            if (lastDist == 0.0) {
                return true; // Avoid division by zero
            }
            
            qreal scaleFactor = currentDist / lastDist;
            qreal newScale = m_scale * scaleFactor;
            newScale = qBound(0.1, newScale, 100.0);
            
            // Calculate center point between the two fingers
            QPointF centerPoint = (currentPos1 + currentPos2) / 2.0;
            
            // Convert to image coordinates
            QPointF imgCenter = mapToImage(centerPoint.toPoint());
            
            // Apply new scale
            m_scale = newScale;
            
            // Adjust offset to zoom toward the center point
            m_offset = (imgCenter * (newScale / m_scale) - centerPoint) * -1.0;
            
            update();
            scheduleUpdate();
            
            return true;
        }  
    }
    
    return QWidget::event(event);
}

void TileView::updateTiles() {
    m_updatePending = false;
    
    // Request an update of the visible tiles
    QRectF visible = visibleImageRect();
    if (!visible.isValid()) {
        return;
    }
    
    // This will trigger the loading of any tiles that are in the visible area
    // The actual loading happens asynchronously in the TileCache
    update();
}

void TileView::animationTick() {
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsed = now - m_animation.startTime;
    
    if (elapsed >= m_animation.duration || m_animation.duration <= 0) {
        // Animation complete
        m_offset = m_animation.targetOffset;
        m_scale = m_animation.targetScale;
        m_animation.active = false;
        m_animationTimer.stop();
        
        updateTileGrid();
        update();
        emit viewChanged();
        return;
    }
    
    // Ease in-out function
    double t = static_cast<double>(elapsed) / m_animation.duration;
    t = t < 0.5 ? 2 * t * t : 1 - qPow(-2 * t + 2, 2) / 2;
    
    // Interpolate between start and target values
    double dx = m_animation.targetOffset.x() - m_animation.startOffset.x();
    double dy = m_animation.targetOffset.y() - m_animation.startOffset.y();
    double ds = m_animation.targetScale - m_animation.startScale;
    
    m_offset = m_animation.startOffset + QPointF(dx * t, dy * t);
    m_scale = m_animation.startScale + ds * t;
    
    updateTileGrid();
    update();
    emit viewChanged();
}

void TileView::updateTileGrid() {
    QRectF visible = visibleImageRect();
    if (!visible.isValid()) {
        return;
    }
    
    int tileSize = 256;  // Should match the tile size in TileCache
    int startX = qFloor(visible.left() / tileSize);
    int startY = qFloor(visible.top() / tileSize);
    int endX = qCeil(visible.right() / tileSize);
    int endY = qCeil(visible.bottom() / tileSize);
    
    QRect newVisibleTileArea(startX, startY, endX - startX + 1, endY - startY + 1);
    
    if (newVisibleTileArea != m_visibleTileArea) {
        m_visibleTileArea = newVisibleTileArea;
        scheduleUpdate();
    }
}

void TileView::startAnimation(const QPointF& targetOffset, double targetScale, int duration) {
    m_animation.startOffset = m_offset;
    m_animation.targetOffset = targetOffset;
    m_animation.startScale = m_scale;
    m_animation.targetScale = targetScale;
    m_animation.startTime = QDateTime::currentMSecsSinceEpoch();
    m_animation.duration = duration;
    m_animation.active = true;
    
    if (!m_animationTimer.isActive()) {
        m_animationTimer.start(16);  // ~60 FPS
    }
}

void TileView::stopAnimation() {
    if (m_animation.active) {
        m_animation.active = false;
        m_animationTimer.stop();
    }
}

void TileView::ensureVisible(const QPointF& imgPos) {
    QPoint viewPos = mapFromImage(imgPos);
    QRect viewRect = viewportRect().toRect();
    
    if (!viewRect.contains(viewPos)) {
        // Calculate the offset needed to make the point visible
        int dx = 0;
        int dy = 0;
        
        if (viewPos.x() < viewRect.left()) {
            dx = viewPos.x() - viewRect.left();
        } else if (viewPos.x() > viewRect.right()) {
            dx = viewPos.x() - viewRect.right();
        }
        
        if (viewPos.y() < viewRect.top()) {
            dy = viewPos.y() - viewRect.top();
        } else if (viewPos.y() > viewRect.bottom()) {
            dy = viewPos.y() - viewRect.bottom();
        }
        
        m_offset += QPointF(dx, dy);
        update();
    }
}

QRectF TileView::imageRect() const {
    QSize imgSize = sourceSize();
    if (imgSize.isEmpty()) {
        return QRectF();
    }
    
    return QRectF(
        m_offset.x(),
        m_offset.y(),
        imgSize.width() * m_scale,
        imgSize.height() * m_scale
    );
}

QRectF TileView::viewportRect() const {
    return QRectF(0, 0, width(), height());
}

void TileView::scheduleUpdate() {
    if (!m_updatePending) {
        m_updatePending = true;
        m_updateTimer.start();
    }
}

} // namespace ui
} // namespace airphoto_viewer
