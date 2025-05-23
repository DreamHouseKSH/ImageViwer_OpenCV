import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import AirPhoto.Viewer.Measurement 1.0

/**
 * Main QML file for the Measurement Example application.
 * 
 * This application demonstrates how to use the MeasurementTool and MeasurementOverlay
 * components in a real-world scenario.
 */

ApplicationWindow {
    id: window
    
    // Window properties
    width: 1024
    height: 768
    minimumWidth: 800
    minimumHeight: 600
    visible: true
    title: qsTr("AirPhoto Viewer - Measurement Tool Example")
    
    // Application font
    font.pixelSize: 14
    
    // Measurement tool instance
    MeasurementTool {
        id: measurementTool
        // Set the pixel scale (1 pixel = 0.1 meters in this example)
        pixelScale: 0.1
        
        // Handle measurement completion
        onMeasurementCompleted: {
            if (mode === MeasurementTool.Distance) {
                statusBar.text = qsTr("Distance measured: %1 meters").arg(distance.toFixed(2))
            } else if (mode === MeasurementTool.Area) {
                statusBar.text = qsTr("Area measured: %1 square meters").arg(area.toFixed(2))
            }
        }
    }
    
    // Main layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        
        // Toolbar
        ToolBar {
            id: toolbar
            Layout.fillWidth: true
            
            RowLayout {
                anchors.fill: parent
                spacing: 10
                
                // Tool group for measurement modes
                ButtonGroup {
                    id: toolGroup
                    buttons: [distanceButton, areaButton]
                }
                
                // Distance measurement button
                ToolButton {
                    id: distanceButton
                    text: qsTr("Distance")
                    icon.source: "qrc:/icons/ruler.svg"
                    checkable: true
                    checked: true
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Measure distance between points (D)")
                    
                    onClicked: {
                        measurementTool.mode = MeasurementTool.Distance
                        measurementTool.clear()
                        statusBar.text = qsTr("Click to start measuring distance")
                    }
                }
                
                // Area measurement button
                ToolButton {
                    id: areaButton
                    text: qsTr("Area")
                    icon.source: "qrc:/icons/shape-polygon.svg"
                    checkable: true
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Measure area of a polygon (A)")
                    
                    onClicked: {
                        measurementTool.mode = MeasurementTool.Area
                        measurementTool.clear()
                        statusBar.text = qsTr("Click to start measuring area")
                    }
                }
                
                // Separator
                Rectangle {
                    width: 1
                    Layout.fillHeight: true
                    color: palette.mid
                    opacity: 0.5
                }
                
                // Clear button
                ToolButton {
                    id: clearButton
                    text: qsTr("Clear")
                    icon.source: "qrc:/icons/eraser.svg"
                    enabled: measurementTool.points.length > 0
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Clear measurements (Delete)")
                    
                    onClicked: {
                        measurementTool.clear()
                        statusBar.text = qsTr("Measurements cleared")
                    }
                }
                
                // Spacer
                Item { Layout.fillWidth: true }
                
                // Pixel scale input
                Label {
                    text: qsTr("Scale:")
                    verticalAlignment: Text.AlignVCenter
                }
                
                TextField {
                    id: scaleInput
                    text: (measurementTool.pixelScale * 100).toFixed(2)
                    validator: DoubleValidator { bottom: 0.01; top: 1000.0; decimals: 2 }
                    implicitWidth: 80
                    
                    onAccepted: {
                        var scale = parseFloat(text) / 100.0
                        if (!isNaN(scale) && scale > 0) {
                            measurementTool.pixelScale = scale
                        } else {
                            text = (measurementTool.pixelScale * 100).toFixed(2)
                        }
                    }
                    
                    ToolTip {
                        text: qsTr("Pixels per meter (×100)")
                        visible: scaleInput.hovered
                    }
                }
                
                Label {
                    text: qsTr("cm/px")
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
        
        // Main content area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: palette.window
            
            // Background grid
            Grid {
                anchors.fill: parent
                columns: 20
                rows: 20
                spacing: 0
                
                Repeater {
                    model: parent.columns * parent.rows
                    Rectangle {
                        width: parent.width / parent.columns
                        height: parent.height / parent.rows
                        color: index % 2 === 0 ? "#f0f0f0" : "#e0e0e0"
                        border.color: "#d0d0d0"
                        border.width: 0.5
                    }
                }
            }
            
            // Measurement overlay
            MeasurementOverlay {
                id: overlay
                anchors.fill: parent
                tool: measurementTool
                lineColor: "#2c7be5"
                lineWidth: 2
                textColor: "#ffffff"
                textSize: 12
                
                // Handle double click to complete measurement
                onDoubleClicked: {
                    if (measurementTool.mode === MeasurementTool.Distance && measurementTool.points.length >= 2) {
                        statusBar.text = qsTr("Distance: %1 meters").arg(measurementTool.distance.toFixed(2))
                    } else if (measurementTool.mode === MeasurementTool.Area && measurementTool.points.length >= 3) {
                        statusBar.text = qsTr("Area: %1 square meters").arg(measurementTool.area.toFixed(2))
                    }
                }
                
                // Update status bar on point addition/removal
                Connections {
                    target: measurementTool
                    function onPointsChanged() {
                        clearButton.enabled = measurementTool.points.length > 0
                        
                        if (measurementTool.points.length === 0) {
                            if (measurementTool.mode === MeasurementTool.Distance) {
                                statusBar.text = qsTr("Click to start measuring distance")
                            } else if (measurementTool.mode === MeasurementTool.Area) {
                                statusBar.text = qsTr("Click to start measuring area")
                            }
                        } else {
                            if (measurementTool.mode === MeasurementTool.Distance) {
                                statusBar.text = qsTr("Click to add point, double-click to finish")
                            } else if (measurementTool.mode === MeasurementTool.Area) {
                                statusBar.text = qsTr("Click to add point, double-click to complete polygon")
                            }
                        }
                    }
                }
            }
            
            // Instructions overlay
            Rectangle {
                anchors {
                    top: parent.top
                    right: parent.right
                    margins: 20
                }
                width: 250
                height: instructionsColumn.implicitHeight + 20
                color: "#e0f0ff"
                border.color: "#2c7be5"
                border.width: 1
                radius: 5
                opacity: 0.9
                visible: instructionsVisible.checked
                
                Column {
                    id: instructionsColumn
                    anchors {
                        fill: parent
                        margins: 10
                    }
                    spacing: 5
                    
                    Label {
                        text: qsTr("Instructions")
                        font.bold: true
                        font.pixelSize: 14
                        color: "#2c7be5"
                    }
                    
                    Label {
                        text: qsTr("• Click to add measurement points")
                        visible: measurementTool.mode === MeasurementTool.Distance || measurementTool.mode === MeasurementTool.Area
                    }
                    
                    Label {
                        text: qsTr("• Double-click to complete measurement")
                        visible: measurementTool.mode === MeasurementTool.Distance || measurementTool.mode === MeasurementTool.Area
                    }
                    
                    Label {
                        text: qsTr("• Right-click or press Delete to remove last point")
                    }
                    
                    Label {
                        text: qsTr("• Press Esc to clear all points")
                    }
                }
            }
            
            // Toggle instructions button
            ToolButton {
                id: instructionsVisible
                anchors {
                    top: parent.top
                    right: parent.right
                    margins: 10
                }
                icon.source: "qrc:/icons/info-circle.svg"
                checkable: true
                checked: true
                ToolTip.visible: hovered
                ToolTip.text: checked ? qsTr("Hide instructions") : qsTr("Show instructions")
            }
        }
        
        // Status bar
        StatusBar {
            id: statusBar
            Layout.fillWidth: true
            
            Label {
                id: statusLabel
                text: qsTr("Select a measurement tool to begin")
            }
        }
    }
    
    // Handle keyboard shortcuts
    Shortcut {
        sequence: StandardKey.Delete
        onActivated: {
            if (measurementTool.points.length > 0) {
                measurementTool.removeLastPoint()
                if (measurementTool.points.length === 0) {
                    statusBar.text = qsTr("Last point removed")
                }
            }
        }
    }
    
    Shortcut {
        sequence: StandardKey.Cancel
        onActivated: {
            if (measurementTool.points.length > 0) {
                measurementTool.clear()
                statusBar.text = qsTr("Measurements cleared")
            }
        }
    }
    
    Shortcut {
        sequence: "D"
        onActivated: distanceButton.clicked()
    }
    
    Shortcut {
        sequence: "A"
        onActivated: areaButton.clicked()
    }
    
    // Set default focus
    Component.onCompleted: {
        forceActiveFocus()
    }
}
