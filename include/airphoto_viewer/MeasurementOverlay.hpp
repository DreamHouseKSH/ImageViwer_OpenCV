#ifndef MEASUREMENT_OVERLAY_HPP
#define MEASUREMENT_OVERLAY_HPP

#include <QQuickPaintedItem>
#include <QPainterPath>
#include <QColor>
#include "MeasurementTool.hpp"

namespace airphoto {

/**
 * @class MeasurementOverlay
 * @brief 이미지 뷰어에 측정 도구를 오버레이로 그리는 QML 아이템
 * 
 * 이 클래스는 QQuickPaintedItem을 상속받아 QML에서 사용할 수 있으며,
 * MeasurementTool과 연동되어 거리 및 면적 측정 결과를 시각화합니다.
 */
class MeasurementOverlay : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(MeasurementTool* tool READ tool WRITE setTool NOTIFY toolChanged)
    Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor NOTIFY lineColorChanged)
    Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(int textSize READ textSize WRITE setTextSize NOTIFY textSizeChanged)

public:
    explicit MeasurementOverlay(QQuickItem *parent = nullptr);
    ~MeasurementOverlay() override;

    // QQuickPaintedItem interface
    void paint(QPainter *painter) override;

    // Property getters
    MeasurementTool* tool() const;
    QColor lineColor() const;
    int lineWidth() const;
    QColor textColor() const;
    int textSize() const;

public slots:
    // Property setters
    void setTool(MeasurementTool *tool);
    void setLineColor(const QColor &color);
    void setLineWidth(int width);
    void setTextColor(const QColor &color);
    void setTextSize(int size);

protected:
    // Mouse event handlers
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void toolChanged();
    void lineColorChanged();
    void lineWidthChanged();
    void textColorChanged();
    void textSizeChanged();

private slots:
    void updateOverlay();

private:
    /**
     * @brief 측정선을 그립니다.
     * @param painter QPainter 인스턴스
     * @param points 측정 지점들
     * @param isComplete 측정이 완료되었는지 여부
     */
    void drawMeasurementLine(QPainter *painter, const QVector<QPointF> &points, bool isComplete);

    /**
     * @brief 측정 텍스트를 그립니다.
     * @param painter QPainter 인스턴스
     * @param points 측정 지점들
     * @param isComplete 측정이 완료되었는지 여부
     */
    void drawMeasurementText(QPainter *painter, const QVector<QPointF> &points, bool isComplete);

    /**
     * @brief 마우스 커서 위치에 대한 피드백을 그립니다.
     * @param painter QPainter 인스턴스
     * @param pos 마우스 커서 위치
     */
    void drawHoverFeedback(QPainter *painter, const QPointF &pos);

    /**
     * @brief 픽셀 좌표를 뷰포트 좌표로 변환합니다.
     * @param point 변환할 좌표
     * @return 변환된 좌표
     */
    QPointF mapToViewport(const QPointF &point) const;

    MeasurementTool *m_tool{nullptr};     ///< 측정 도구 인스턴스
    QColor m_lineColor{Qt::red};          ///< 측정선 색상
    int m_lineWidth{2};                   ///< 측정선 두께
    QColor m_textColor{Qt::white};        ///< 측정 텍스트 색상
    int m_textSize{10};                   ///< 측정 텍스트 크기
    QPointF m_hoverPos;                   ///< 현재 마우스 호버 위치
    bool m_isHovering{false};             ///< 마우스 호버 중인지 여부
};

} // namespace airphoto

#endif // MEASUREMENT_OVERLAY_HPP
