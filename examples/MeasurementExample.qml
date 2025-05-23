import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import AirPhoto.Viewer.Measurement 1.0

ApplicationWindow {
    id: window
    width: 1024
    height: 768
    visible: true
    title: qsTr("항공사진 뷰어 - 측정 도구 예제")

    // 측정 도구 인스턴스
    MeasurementTool {
        id: measurementTool
        pixelScale: 0.1 // 1픽셀 = 0.1미터 (예시)
    }


    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        // 툴바
        Rectangle {
            Layout.fillWidth: true
            height: 50
            color: "#f0f0f0"
            
            Row {
                anchors.centerIn: parent
                spacing: 10

                ButtonGroup {
                    id: toolGroup
                    buttons: [distanceButton, areaButton]
                }


                Button {
                    id: distanceButton
                    text: qsTr("거리 측정")
                    checkable: true
                    checked: true
                    onClicked: {
                        measurementTool.mode = MeasurementTool.Mode.Distance
                        clearButton.enabled = measurementTool.points.length > 0
                    }
                }


                Button {
                    id: areaButton
                    text: qsTr("면적 측정")
                    checkable: true
                    onClicked: {
                        measurementTool.mode = MeasurementTool.Mode.Area
                        clearButton.enabled = measurementTool.points.length > 0
                    }
                }


                Button {
                    id: clearButton
                    text: qsTr("지우기")
                    enabled: false
                    onClicked: measurementTool.clear()
                }


                Item { Layout.fillWidth: true }

                Label {
                    text: {
                        if (measurementTool.mode === MeasurementTool.Mode.Distance && measurementTool.points.length >= 2) {
                            return qsTr("거리: %1 미터").arg(measurementTool.distance.toFixed(2))
                        } else if (measurementTool.mode === MeasurementTool.Mode.Area && measurementTool.points.length >= 3) {
                            return qsTr("면적: %1 제곱미터").arg(measurementTool.area.toFixed(2))
                        } else {
                            return qsTr("이미지를 클릭하여 측정을 시작하세요.")
                        }
                    }
                    font.bold: true
                }
            }
        }
        // 이미지 뷰어 영역
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#333"
            
            // 이미지 배경 (그리드)
            Rectangle {
                id: background
                anchors.fill: parent
                color: "#333"
                clip: true

                // 간단한 그리드 패턴
                Canvas {
                    id: gridCanvas
                    anchors.fill: parent
                    renderStrategy: Canvas.Threaded
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.strokeStyle = "#444"
                        ctx.lineWidth = 1
                        
                        var gridSize = 20
                        
                        // 수직선 그리기
                        for (var x = 0; x <= width; x += gridSize) {
                            ctx.beginPath()
                            ctx.moveTo(x, 0)
                            ctx.lineTo(x, height)
                            ctx.stroke()
                        }
                        
                        // 수평선 그리기
                        for (var y = 0; y <= height; y += gridSize) {
                            ctx.beginPath()
                            ctx.moveTo(0, y)
                            ctx.lineTo(width, y)
                            ctx.stroke()
                        }
                    }
                }

                // 측정 오버레이
                MeasurementOverlay {
                    id: overlay
                    anchors.fill: parent
                    tool: measurementTool
                    lineColor: "#ff0000"
                    lineWidth: 2
                    textColor: "#ffffff"
                    textSize: 12

                    // 마우스 더블 클릭 시 측정 완료
                    onDoubleClicked: {
                        if (measurementTool.mode === MeasurementTool.Mode.Distance && measurementTool.points.length >= 2) {
                            // 거리 측정 완료
                            console.log("Distance measurement complete:", measurementTool.distance)
                        } else if (measurementTool.mode === MeasurementTool.Mode.Area && measurementTool.points.length >= 3) {
                            // 면적 측정 완료
                            console.log("Area measurement complete:", measurementTool.area)
                        }
                        clearButton.enabled = false
                    }

                    // 점이 추가될 때마다 상태 업데이트
                    Connections {
                        target: measurementTool
                        function onMeasurementUpdated() {
                            clearButton.enabled = measurementTool.points.length > 0
                        }
                    }
                }
            }
        }
    }
    
    // 단축키 처리
    Shortcut {
        sequence: "Esc"
        onActivated: {
            measurementTool.clear()
            clearButton.enabled = false
        }
    }
    
    Shortcut {
        sequence: "Delete"
        onActivated: {
            if (measurementTool.points.length > 0) {
                measurementTool.removeLastPoint()
                clearButton.enabled = measurementTool.points.length > 0
            }
        }
    }
}
