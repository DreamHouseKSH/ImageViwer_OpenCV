#include "airphoto_viewer/MeasurementTool.hpp"
#include <QDebug>
#include <cmath>

namespace airphoto {

MeasurementTool::MeasurementTool(QObject *parent)
    : QObject(parent)
{
}

MeasurementTool::~MeasurementTool() = default;

void MeasurementTool::setMode(Mode mode)
{
    if (m_mode != mode) {
        m_mode = mode;
        clear();
        emit measurementUpdated();
    }
}

MeasurementTool::Mode MeasurementTool::mode() const
{
    return m_mode;
}

void MeasurementTool::addPoint(const QPointF &point)
{
    if (m_mode == Mode::None) {
        qWarning() << "MeasurementTool: Cannot add point - no measurement mode set";
        return;
    }

    m_points.append(point);
    emit measurementUpdated();
}

void MeasurementTool::removeLastPoint()
{
    if (!m_points.isEmpty()) {
        m_points.removeLast();
        emit measurementUpdated();
    }
}

void MeasurementTool::clear()
{
    if (!m_points.isEmpty()) {
        m_points.clear();
        emit measurementUpdated();
    }
}

double MeasurementTool::distance() const
{
    if (m_points.size() < 2) {
        return 0.0;
    }

    double totalDistance = 0.0;
    for (int i = 1; i < m_points.size(); ++i) {
        totalDistance += calculateDistance(m_points[i-1], m_points[i]);
    }
    
    return totalDistance * m_pixelScale;
}

double MeasurementTool::area() const
{
    if (m_points.size() < 3) {
        return 0.0;
    }
    
    double area = calculatePolygonArea(m_points);
    return std::abs(area) * m_pixelScale * m_pixelScale;
}

const QVector<QPointF> &MeasurementTool::points() const
{
    return m_points;
}

void MeasurementTool::setLineColor(const QColor &color)
{
    if (m_lineColor != color) {
        m_lineColor = color;
        emit measurementUpdated();
    }
}

void MeasurementTool::setLineWidth(int width)
{
    if (m_lineWidth != width && width > 0) {
        m_lineWidth = width;
        emit measurementUpdated();
    }
}

void MeasurementTool::setTextColor(const QColor &color)
{
    if (m_textColor != color) {
        m_textColor = color;
        emit measurementUpdated();
    }
}

void MeasurementTool::setTextSize(int size)
{
    if (m_textSize != size && size > 0) {
        m_textSize = size;
        emit measurementUpdated();
    }
}

void MeasurementTool::setPixelScale(double scale)
{
    if (!qFuzzyCompare(m_pixelScale, scale) && scale > 0) {
        m_pixelScale = scale;
        emit measurementUpdated();
    }
}

double MeasurementTool::pixelScale() const
{
    return m_pixelScale;
}

double MeasurementTool::calculateDistance(const QPointF &p1, const QPointF &p2) const
{
    QPointF diff = p2 - p1;
    return std::sqrt(diff.x() * diff.x() + diff.y() * diff.y());
}

double MeasurementTool::calculatePolygonArea(const QVector<QPointF> &points) const
{
    // 신발끈 공식(Shoelace formula)을 사용하여 다각형의 면적 계산
    double area = 0.0;
    int n = points.size();
    
    for (int i = 0; i < n; ++i) {
        const QPointF &p1 = points[i];
        const QPointF &p2 = points[(i + 1) % n];
        area += (p1.x() * p2.y()) - (p2.x() * p1.y());
    }
    
    return area / 2.0;
}

} // namespace airphoto
