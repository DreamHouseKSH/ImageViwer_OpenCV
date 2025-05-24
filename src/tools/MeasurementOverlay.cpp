#include "airphoto_viewer/MeasurementOverlay.hpp"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QtMath>

namespace airphoto {

MeasurementOverlay::MeasurementOverlay(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_lineColor(Qt::red)
    , m_lineWidth(2)
    , m_textColor(Qt::white)
    , m_textSize(10)
    , m_isHovering(false)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setAcceptHoverEvents(true);
    setAcceptTouchEvents(true);
    setFlag(ItemHasContents, true);
    setFlag(ItemAcceptsInputMethod, true);
    setAntialiasing(true);
}

MeasurementOverlay::~MeasurementOverlay() = default;

void MeasurementOverlay::paint(QPainter *painter)
{
    if (!painter || !m_tool) {
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    const auto &points = m_tool->points();
    const bool isComplete = (m_tool->mode() == MeasurementTool::Mode::Distance && points.size() >= 2) ||
                           (m_tool->mode() == MeasurementTool::Mode::Area && points.size() >= 3);

    // 측정선 그리기
    drawMeasurementLine(painter, points, isComplete);

    // 측정 텍스트 그리기
    if (!points.isEmpty()) {
        drawMeasurementText(painter, points, isComplete);
    }

    // 호버 피드백 그리기
    if (m_isHovering) {
        drawHoverFeedback(painter, m_hoverPos);
    }
}

MeasurementTool *MeasurementOverlay::tool() const
{
    return m_tool;
}

QColor MeasurementOverlay::lineColor() const
{
    return m_lineColor;
}

int MeasurementOverlay::lineWidth() const
{
    return m_lineWidth;
}

QColor MeasurementOverlay::textColor() const
{
    return m_textColor;
}

int MeasurementOverlay::textSize() const
{
    return m_textSize;
}

void MeasurementOverlay::setTool(MeasurementTool *tool)
{
    if (m_tool == tool) {
        return;
    }

    if (m_tool) {
        disconnect(m_tool, &MeasurementTool::measurementUpdated, this, &MeasurementOverlay::updateOverlay);
    }

    m_tool = tool;

    if (m_tool) {
        connect(m_tool, &MeasurementTool::measurementUpdated, this, &MeasurementOverlay::updateOverlay);
    }

    emit toolChanged();
    update();
}

void MeasurementOverlay::setLineColor(const QColor &color)
{
    if (m_lineColor == color) {
        return;
    }

    m_lineColor = color;
    emit lineColorChanged();
    update();
}

void MeasurementOverlay::setLineWidth(int width)
{
    if (m_lineWidth == width || width < 1) {
        return;
    }

    m_lineWidth = width;
    emit lineWidthChanged();
    update();
}

void MeasurementOverlay::setTextColor(const QColor &color)
{
    if (m_textColor == color) {
        return;
    }

    m_textColor = color;
    emit textColorChanged();
    update();
}

void MeasurementOverlay::setTextSize(int size)
{
    if (m_textSize == size || size < 1) {
        return;
    }

    m_textSize = size;
    emit textSizeChanged();
    update();
}

void MeasurementOverlay::mousePressEvent(QMouseEvent *event)
{
    if (!m_tool || event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    event->accept();
    setFocus(true);

    // 마우스 위치를 뷰포트 좌표로 변환하여 추가
    const QPointF pos = mapToViewport(event->localPos());
    m_tool->addPoint(pos);
}

void MeasurementOverlay::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_tool) {
        event->ignore();
        return;
    }

    event->accept();
    m_hoverPos = event->localPos();
    update();
}

void MeasurementOverlay::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_tool) {
        event->ignore();
        return;
    }

    event->accept();
    
    // 더블 클릭 시 현재 측정 완료 및 새 측정 시작
    m_tool->clear();
}

void MeasurementOverlay::hoverMoveEvent(QHoverEvent *event)
{
    if (!m_tool) {
        event->ignore();
        return;
    }

    event->accept();
    m_hoverPos = event->posF();
    m_isHovering = true;
    update();
}

void MeasurementOverlay::keyPressEvent(QKeyEvent *event)
{
    if (!m_tool) {
        event->ignore();
        return;
    }

    switch (event->key()) {
    case Qt::Key_Escape:
        // ESC 키로 측정 취소
        m_tool->clear();
        event->accept();
        break;
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
        // Backspace/Delete 키로 마지막 점 제거
        m_tool->removeLastPoint();
        event->accept();
        break;
    default:
        event->ignore();
        break;
    }
}

void MeasurementOverlay::updateOverlay()
{
    update();
}

void MeasurementOverlay::drawMeasurementLine(QPainter *painter, const QVector<QPointF> &points, bool isComplete)
{
    if (points.isEmpty()) {
        return;
    }

    QPen pen(m_lineColor);
    pen.setWidth(m_lineWidth);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);

    // 점 그리기
    for (const auto &point : points) {
        const QPointF p = mapToViewport(point);
        painter->drawEllipse(p, m_lineWidth * 1.5, m_lineWidth * 1.5);
    }

    // 선 그리기
    if (points.size() > 1) {
        QPainterPath path;
        path.moveTo(mapToViewport(points.first()));

        for (int i = 1; i < points.size(); ++i) {
            path.lineTo(mapToViewport(points[i]));
        }

        // 면적 모드이고 3개 이상의 점이 있으면 닫힌 도형으로 그림
        if (m_tool->mode() == MeasurementTool::Mode::Area && points.size() >= 3 && isComplete) {
            path.closeSubpath();
            
            // 면적 채우기 (반투명)
            QColor fillColor = m_lineColor;
            fillColor.setAlpha(50);
            painter->fillPath(path, fillColor);
        }

        painter->drawPath(path);
    }
}

void MeasurementOverlay::drawMeasurementText(QPainter *painter, const QVector<QPointF> &points, bool isComplete)
{
    if (points.isEmpty()) {
        return;
    }

    QFont font = painter->font();
    font.setPixelSize(m_textSize);
    painter->setFont(font);
    painter->setPen(m_textColor);

    // 거리 또는 면적 텍스트
    QString text;
    if (m_tool->mode() == MeasurementTool::Mode::Distance) {
        if (points.size() >= 2 && isComplete) {
            text = QString("거리: %1 미터").arg(m_tool->distance(), 0, 'f', 2);
        }
    } else if (m_tool->mode() == MeasurementTool::Mode::Area) {
        if (points.size() >= 3 && isComplete) {
            text = QString("면적: %1 제곱미터").arg(m_tool->area(), 0, 'f', 2);
        }
    }

    if (!text.isEmpty()) {
        // 텍스트 배경 (가독성 향상을 위해)
        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(text).adjusted(-4, -2, 4, 2);
        textRect.moveCenter(QPoint(width() / 2, 20)); // 상단 중앙에 배치
        
        painter->fillRect(textRect, QColor(0, 0, 0, 128));
        painter->drawText(textRect, Qt::AlignCenter, text);
    }

    // 각 점 사이의 거리 표시
    if (m_tool->mode() == MeasurementTool::Mode::Distance && points.size() > 1) {
        for (int i = 1; i < points.size(); ++i) {
            const QPointF p1 = points[i-1];
            const QPointF p2 = points[i];
            const QPointF mid = (mapToViewport(p1) + mapToViewport(p2)) / 2;
            
            // Use the public method to calculate distance
            const double dist = m_tool->distanceBetween(p1, p2) * m_tool->pixelScale();
            const QString distText = QString("%1m").arg(dist, 0, 'f', 1);
            
            QRectF textRect = painter->fontMetrics().boundingRect(distText);
            textRect.moveCenter(mid.toPoint());
            textRect.adjust(-4, -2, 4, 2);
            
            painter->fillRect(textRect, QColor(0, 0, 0, 128));
            painter->drawText(textRect, Qt::AlignCenter, distText);
        }
    }
}

void MeasurementOverlay::drawHoverFeedback(QPainter *painter, const QPointF &pos)
{
    if (!m_tool || m_tool->points().isEmpty()) {
        return;
    }

    QPen pen(m_lineColor);
    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);

    const QPointF lastPoint = mapToViewport(m_tool->points().last());
    
    // 마지막 점에서 커서까지 점선 그리기
    painter->drawLine(lastPoint, pos);
    
    // 커서 위치에 십자선 그리기
    const int crossSize = 10;
    painter->drawLine(pos - QPointF(crossSize, 0), pos + QPointF(crossSize, 0));
    painter->drawLine(pos - QPointF(0, crossSize), pos + QPointF(0, crossSize));
    
    // 현재까지의 거리 또는 면적 미리보기
    if (m_tool->mode() == MeasurementTool::Mode::Distance && m_tool->points().size() >= 1) {
        // Convert the hover position from viewport to image coordinates
        QPointF imagePos = pos;
        // Use the public method to calculate distance
        const double dist = m_tool->distanceBetween(m_tool->points().last(), imagePos) * m_tool->pixelScale();
        const QString text = QString("%1m").arg(dist, 0, 'f', 1);
        
        QFont font = painter->font();
        font.setPixelSize(m_textSize);
        painter->setFont(font);
        
        QRectF textRect = painter->fontMetrics().boundingRect(text);
        textRect.moveTopLeft(pos.toPoint() + QPoint(10, 10));
        textRect.adjust(-4, -2, 4, 2);
        
        painter->fillRect(textRect, QColor(0, 0, 0, 128));
        painter->setPen(m_textColor);
        painter->drawText(textRect, Qt::AlignCenter, text);
    }
}

QPointF MeasurementOverlay::mapToViewport(const QPointF &point) const
{
    // 뷰포트 좌표계로 변환 (필요한 경우 추가 변환 로직 구현)
    return point;
}

} // namespace airphoto
