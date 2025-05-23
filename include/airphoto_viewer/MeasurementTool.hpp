#ifndef MEASUREMENT_TOOL_HPP
#define MEASUREMENT_TOOL_HPP

#include <QObject>
#include <QVector>
#include <QPointF>
#include <QColor>

namespace airphoto {

/**
 * @class MeasurementTool
 * @brief 이미지 뷰어에서 거리 및 면적 측정을 담당하는 클래스
 * 
 * 이 클래스는 이미지 상에서 거리와 면적을 측정하는 기능을 제공합니다.
 * 측정 지점은 이미지 좌표계를 사용하며, 실제 거리/면적 계산을 위한 스케일 변환을 지원합니다.
 */
class MeasurementTool : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 측정 모드 열거형
     */
    enum class Mode {
        None,       ///< 측정 모드 아님
        Distance,   ///< 거리 측정 모드
        Area        ///< 면적 측정 모드
    };

    /**
     * @brief 생성자
     * @param parent 부모 QObject
     */
    explicit MeasurementTool(QObject *parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~MeasurementTool() override;

    /**
     * @brief 측정 모드를 설정합니다.
     * @param mode 설정할 측정 모드
     */
    void setMode(Mode mode);

    /**
     * @brief 현재 측정 모드를 반환합니다.
     * @return 현재 측정 모드
     */
    Mode mode() const;

    /**
     * @brief 측정 지점을 추가합니다.
     * @param point 이미지 좌표계 상의 점
     */
    void addPoint(const QPointF &point);

    /**
     * @brief 마지막으로 추가된 지점을 제거합니다.
     */
    void removeLastPoint();

    /**
     * @brief 모든 측정 지점을 초기화합니다.
     */
    void clear();

    /**
     * @brief 측정된 거리를 반환합니다.
     * @return 픽셀 단위의 거리 (실제 거리는 스케일에 따라 변환 필요)
     */
    double distance() const;

    /**
     * @brief 측정된 면적을 반환합니다.
     * @return 픽셀 제곱 단위의 면적 (실제 면적은 스케일 제곱에 따라 변환 필요)
     */
    double area() const;

    /**
     * @brief 측정 지점들을 반환합니다.
     * @return 측정 지점들의 벡터
     */
    const QVector<QPointF> &points() const;

    /**
     * @brief 측정 선의 색상을 설정합니다.
     * @param color 설정할 색상
     */
    void setLineColor(const QColor &color);

    /**
     * @brief 측정 선의 두께를 설정합니다.
     * @param width 선 두께 (픽셀 단위)
     */
    void setLineWidth(int width);

    /**
     * @brief 측정 텍스트의 색상을 설정합니다.
     * @param color 설정할 색상
     */
    void setTextColor(const QColor &color);

    /**
     * @brief 측정 텍스트의 글꼴 크기를 설정합니다.
     * @param size 글꼴 크기 (포인트 단위)
     */
    void setTextSize(int size);

    /**
     * @brief 픽셀당 실제 거리 비율을 설정합니다.
     * @param scale 픽셀당 실제 거리 (예: 0.1m/pixel)
     */
    void setPixelScale(double scale);

    /**
     * @brief 현재 설정된 픽셀당 실제 거리 비율을 반환합니다.
     * @return 픽셀당 실제 거리
     */
    double pixelScale() const;

signals:
    /**
     * @brief 측정이 업데이트될 때 발생하는 시그널
     */
    void measurementUpdated();

private:
    /**
     * @brief 두 점 사이의 거리를 계산합니다.
     * @param p1 첫 번째 점
     * @param p2 두 번째 점
     * @return 두 점 사이의 거리
     */
    double calculateDistance(const QPointF &p1, const QPointF &p2) const;

    /**
     * @brief 다각형의 면적을 계산합니다.
     * @param points 다각형의 꼭짓점들
     * @return 다각형의 면적 (신발끈 공식 사용)
     */
    double calculatePolygonArea(const QVector<QPointF> &points) const;

    Mode m_mode{Mode::None};              ///< 현재 측정 모드
    QVector<QPointF> m_points;            ///< 측정 지점들
    QColor m_lineColor{Qt::red};          ///< 측정 선 색상
    int m_lineWidth{2};                   ///< 측정 선 두께
    QColor m_textColor{Qt::white};        ///< 측정 텍스트 색상
    int m_textSize{10};                   ///< 측정 텍스트 크기
    double m_pixelScale{1.0};             ///< 픽셀당 실제 거리 비율
};

} // namespace airphoto

#endif // MEASUREMENT_TOOL_HPP
