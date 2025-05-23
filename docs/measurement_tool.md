# Measurement Tool Module

## 개요
Measurement Tool 모듈은 항공사진 뷰어에서 거리 및 면적 측정 기능을 제공하는 QML 컴포넌트 세트입니다. 이 모듈은 `MeasurementTool`과 `MeasurementOverlay` 두 가지 주요 컴포넌트로 구성되어 있습니다.

## 기능
- **거리 측정**: 두 점 이상을 연결하여 거리 측정
- **면적 측정**: 세 점 이상으로 다각형을 그려 면적 측정
- **실시간 미리보기**: 측정 중 실시간으로 거리/면적 미리보기
- **상호작용**: 마우스/터치 제스처 지원
- **사용자 정의**: 선 색상, 두께, 텍스트 스타일 등 사용자 정의 가능

## 컴포넌트

### 1. MeasurementTool
측정 기능을 제공하는 논리적 컴포넌트입니다.

#### 속성
- **mode**: 측정 모드 (`None`, `Distance`, `Area`)
- **pixelScale**: 픽셀당 실제 거리 (예: 0.1 = 1픽셀당 0.1미터)
- **points**: 측정 지점 배열 (읽기 전용)
- **distance**: 계산된 거리 (미터 단위, 읽기 전용)
- **area**: 계산된 면적 (제곱미터 단위, 읽기 전용)

#### 메서드
- **addPoint(point)**: 측정 지점 추가
- **removeLastPoint()**: 마지막 지점 제거
- **clear()**: 모든 측정 지점 초기화
- **calculateDistance(p1, p2)**: 두 점 사이의 거리 계산

### 2. MeasurementOverlay
측정을 시각적으로 표시하는 QML 아이템입니다.

#### 속성
- **tool**: 연결된 MeasurementTool 인스턴스
- **lineColor**: 측정선 색상 (기본값: 빨강)
- **lineWidth**: 측정선 두께 (기본값: 2)
- **textColor**: 측정 텍스트 색상 (기본값: 흰색)
- **textSize**: 측정 텍스트 크기 (기본값: 12)
- **showHoverFeedback**: 호버 피드백 표시 여부 (기본값: true)

#### 시그널
- **doubleClicked()**: 더블 클릭 시 발생

## 사용 예제

### QML에서 사용하기

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import AirPhoto.Viewer.Measurement 1.0

ApplicationWindow {
    width: 1024
    height: 768
    visible: true
    title: qsTr("Measurement Tool Example")

    // 측정 도구 인스턴스 생성
    MeasurementTool {
        id: measurementTool
        pixelScale: 0.1 // 1픽셀 = 0.1미터
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

                ButtonGroup { id: toolGroup }

                Button {
                    text: qsTr("거리 측정")
                    checkable: true
                    checked: true
                    ButtonGroup.group: toolGroup
                    onClicked: measurementTool.mode = MeasurementTool.Distance
                }


                Button {
                    text: qsTr("면적 측정")
                    checkable: true
                    ButtonGroup.group: toolGroup
                    onClicked: measurementTool.mode = MeasurementTool.Area
                }


                Button {
                    text: qsTr("지우기")
                    enabled: measurementTool.points.length > 0
                    onClicked: measurementTool.clear()
                }


                Item { Layout.fillWidth: true }

                Label {
                    text: {
                        if (measurementTool.mode === MeasurementTool.Distance && measurementTool.points.length >= 2) {
                            return qsTr("거리: %1 미터").arg(measurementTool.distance.toFixed(2))
                        } else if (measurementTool.mode === MeasurementTool.Area && measurementTool.points.length >= 3) {
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
            
            // 측정 오버레이
            MeasurementOverlay {
                anchors.fill: parent
                tool: measurementTool
                lineColor: "red"
                lineWidth: 2
                textColor: "white"
                textSize: 12
                
                onDoubleClicked: {
                    // 측정 완료 시 추가 동작
                    console.log("Measurement completed")
                }
            }
        }
    }
    
    // 단축키 설정
    Shortcut {
        sequence: "Esc"
        onActivated: measurementTool.clear()
    }
    
    Shortcut {
        sequence: "Delete"
        onActivated: measurementTool.removeLastPoint()
    }
}
```

## 빌드 및 실행

### 의존성
- Qt 6.0 이상
- C++17 호환 컴파일러

### 빌드 방법

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### 예제 실행

```bash
./measurement_example
```

## 라이선스
이 프로젝트는 MIT 라이선스 하에 배포됩니다. 자세한 내용은 LICENSE 파일을 참조하세요.

## 기여 방법
버그 리포트, 기능 요청, 풀 리퀘스트는 언제든지 환영합니다. 자세한 내용은 CONTRIBUTING.md를 참조하세요.
