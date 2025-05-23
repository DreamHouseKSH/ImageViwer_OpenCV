#include "airphoto_viewer/MeasurementPlugin.hpp"
#include "airphoto_viewer/MeasurementTool.hpp"
#include "airphoto_viewer/MeasurementOverlay.hpp"

#include <QtQml/qqml.h>

namespace airphoto {

void MeasurementPlugin::registerTypes(const char *uri)
{
    // @uri AirPhoto.Viewer.Measurement
    qmlRegisterType<MeasurementTool>(uri, 1, 0, "MeasurementTool");
    qmlRegisterType<MeasurementOverlay>(uri, 1, 0, "MeasurementOverlay");
    
    qRegisterMetaType<MeasurementTool::Mode>("MeasurementTool.Mode");
    
    // Register enums
    qmlRegisterUncreatableType<MeasurementTool>(
        uri, 1, 0, "MeasurementTool", 
        "MeasurementTool is an abstract type that cannot be instantiated in QML");
}

} // namespace airphoto
