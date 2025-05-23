#ifndef MEASUREMENT_PLUGIN_HPP
#define MEASUREMENT_PLUGIN_HPP

#include <QQmlExtensionPlugin>

namespace airphoto {

/**
 * @class MeasurementPlugin
 * @brief QML에 측정 도구 관련 클래스들을 등록하는 플러그인
 * 
 * 이 플러그인은 MeasurementTool과 MeasurementOverlay를 QML에서 사용할 수 있게 합니다.
 */
class MeasurementPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    /**
     * @brief QML 엔진에 타입을 등록합니다.
     * @param uri 모듈 URI
     */
    void registerTypes(const char *uri) override;
};

} // namespace airphoto

#endif // MEASUREMENT_PLUGIN_HPP
