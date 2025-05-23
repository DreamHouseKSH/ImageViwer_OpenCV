/**
 * Measurement Tool Example
 * 
 * This example demonstrates how to use the MeasurementTool and MeasurementOverlay
 * in a C++ application with Qt.
 */

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>

#include <airphoto_viewer/MeasurementTool.hpp>
#include <airphoto_viewer/MeasurementOverlay.hpp>

int main(int argc, char *argv[])
{
    // Set application attributes
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    // Create the application
    QApplication app(argc, argv);
    app.setOrganizationName("AirPhotoViewer");
    app.setApplicationName("Measurement Example");
    app.setWindowIcon(QIcon(":/icons/app-icon.png"));
    
    // Register C++ types with QML
    qmlRegisterType<MeasurementTool>("AirPhoto.Viewer.Measurement", 1, 0, "MeasurementTool");
    qmlRegisterType<MeasurementOverlay>("AirPhoto.Viewer.Measurement", 1, 0, "MeasurementOverlay");
    
    // Set the style (optional)
    QQuickStyle::setStyle("Material");
    
    // Create and configure the QML engine
    QQmlApplicationEngine engine;
    
    // Load the main QML file
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    
    // Handle QML loading errors
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    
    // Load the QML file
    engine.load(url);
    
    // Check if the QML was loaded successfully
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML file:" << url;
        return -1;
    }
    
    // Run the application
    return app.exec();
}
