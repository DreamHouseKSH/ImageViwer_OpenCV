import QtQuick 2.15
import QtQuick.Shapes 1.15

/**
 * MeasurementOverlay QML Type
 * 
 * A visual overlay for displaying measurements on an image or map.
 * 
 * This component renders measurement lines, points, and text based on the
 * provided MeasurementTool instance. It handles user interaction for creating
 * and modifying measurements.
 * 
 * Example usage:
 * 
 * import AirPhoto.Viewer.Measurement 1.0
 * 
 * MeasurementOverlay {
 *     anchors.fill: parent
 *     tool: measurementTool
 *     lineColor: "red"
 *     lineWidth: 2
 *     textColor: "white"
 *     textSize: 12
 * }
 */
Item {
    id: root
    
    /**
     * The MeasurementTool instance to visualize
     */
    property var tool: null
    
    /**
     * Color of the measurement lines
     */
    property color lineColor: "red"
    
    /**
     * Width of the measurement lines
     */
    property int lineWidth: 2
    
    /**
     * Color of the measurement text
     */
    property color textColor: "white"
    
    /**
     * Size of the measurement text
     */
    property int textSize: 12
    
    /**
     * Whether to show the hover feedback (cursor tracking)
     */
    property bool showHoverFeedback: true
    
    /**
     * Current mouse position in local coordinates
     */
    property point mousePos: Qt.point(0, 0)
    
    /**
     * Whether the mouse is currently pressed
     */
    property bool mousePressed: false
    
    /**
     * Emitted when the user double-clicks to complete a measurement
     */
    signal doubleClicked()
    
    // Update mouse position on hover
    HoverHandler {
        id: hoverHandler
        target: root
        onHoveredChanged: if (hovered) updateMousePosition()
        onPoint: updateMousePosition()
        
        function updateMousePosition() {
            if (hovered) {
                root.mousePos = Qt.point(point.position.x, point.position.y)
            }
        }
    }
    
    // Handle mouse clicks
    TapHandler {
        acceptedButtons: Qt.LeftButton
        onTapped: {
            if (root.tool && root.tool.mode !== MeasurementTool.None) {
                root.tool.addPoint({"x": root.mousePos.x, "y": root.mousePos.y})
            }
        }
        
        onDoubleTapped: {
            root.doubleClicked()
        }
    }
    
    // Handle right-click to remove last point
    TapHandler {
        acceptedButtons: Qt.RightButton
        onTapped: {
            if (root.tool) {
                root.tool.removeLastPoint()
            }
        }
    }
    
    // Visual elements
    Repeater {
        model: root.tool ? root.tool.points.length : 0
        
        // Draw points
        Rectangle {
            x: root.tool.points[index].x - radius
            y: root.tool.points[index].y - radius
            width: radius * 2
            height: radius * 2
            radius: Math.max(4, root.lineWidth * 1.5)
            color: root.lineColor
            border.color: Qt.darker(root.lineColor, 1.5)
            border.width: 1
        }
        
        // Draw lines between points
        Canvas {
            id: lineCanvas
            anchors.fill: parent
            contextType: "2d"
            
            onPaint: {
                if (!root.tool || root.tool.points.length <= 1 || index === 0) {
                    return;
                }
                
                var ctx = getContext("2d");
                ctx.strokeStyle = root.lineColor;
                ctx.lineWidth = root.lineWidth;
                ctx.lineCap = "round";
                ctx.lineJoin = "round";
                
                var p1 = root.tool.points[index-1];
                var p2 = root.tool.points[index];
                
                ctx.beginPath();
                ctx.moveTo(p1.x, p1.y);
                ctx.lineTo(p2.x, p2.y);
                ctx.stroke();
                
                // Draw distance text for line segments
                if (root.tool.mode === MeasurementTool.Distance) {
                    var midX = (p1.x + p2.x) / 2;
                    var midY = (p1.y + p2.y) / 2;
                    var dist = root.tool.calculateDistance(p1, p2);
                    var text = dist.toFixed(2) + " m";
                    
                    ctx.font = root.textSize + "px sans-serif";
                    var textWidth = ctx.measureText(text).width;
                    var textHeight = root.textSize;
                    
                    // Draw background
                    ctx.fillStyle = "rgba(0, 0, 0, 0.7)";
                    ctx.fillRect(
                        midX - textWidth/2 - 2,
                        midY - textHeight - 2,
                        textWidth + 4,
                        textHeight + 4
                    );
                    
                    // Draw text
                    ctx.fillStyle = root.textColor;
                    ctx.textBaseline = "bottom";
                    ctx.fillText(
                        text,
                        midX - textWidth/2,
                        midY - 2
                    );
                }
            }
        }
    }
    
    // Draw polygon fill for area measurements
    Canvas {
        id: areaCanvas
        anchors.fill: parent
        contextType: "2d"
        visible: root.tool && root.tool.mode === MeasurementTool.Area && root.tool.points.length >= 3
        
        onPaint: {
            if (!root.tool || root.tool.points.length < 3) {
                return;
            }
            
            var ctx = getContext("2d");
            
            // Fill polygon
            ctx.fillStyle = Qt.rgba(
                root.lineColor.r,
                root.lineColor.g,
                root.lineColor.b,
                0.2
            );
            
            ctx.beginPath();
            ctx.moveTo(root.tool.points[0].x, root.tool.points[0].y);
            
            for (var i = 1; i < root.tool.points.length; i++) {
                ctx.lineTo(root.tool.points[i].x, root.tool.points[i].y);
            }
            
            ctx.closePath();
            ctx.fill();
            
            // Draw area text
            if (root.tool.points.length >= 3) {
                var area = root.tool.area;
                var text = area.toFixed(2) + " m²";
                
                // Calculate centroid for text placement
                var cx = 0, cy = 0;
                for (var j = 0; j < root.tool.points.length; j++) {
                    cx += root.tool.points[j].x;
                    cy += root.tool.points[j].y;
                }
                cx /= root.tool.points.length;
                cy /= root.tool.points.length;
                
                ctx.font = (root.textSize + 2) + "px sans-serif";
                var textWidth = ctx.measureText(text).width;
                var textHeight = root.textSize + 2;
                
                // Draw background
                ctx.fillStyle = "rgba(0, 0, 0, 0.7)";
                ctx.fillRect(
                    cx - textWidth/2 - 4,
                    cy - textHeight/2 - 2,
                    textWidth + 8,
                    textHeight + 4
                );
                
                // Draw text
                ctx.fillStyle = root.textColor;
                ctx.textBaseline = "middle";
                ctx.textAlign = "center";
                ctx.fillText(text, cx, cy);
            }
        }
    }
    
    // Draw hover feedback (preview line)
    Canvas {
        id: hoverCanvas
        anchors.fill: parent
        contextType: "2d"
        visible: showHoverFeedback && root.tool && root.tool.mode !== MeasurementTool.None && root.tool.points.length > 0
        
        onPaint: {
            if (!root.tool || root.tool.points.length === 0) {
                return;
            }
            
            var ctx = getContext("2d");
            var lastPoint = root.tool.points[root.tool.points.length - 1];
            
            // Draw dashed line from last point to cursor
            ctx.strokeStyle = root.lineColor;
            ctx.lineWidth = root.lineWidth;
            ctx.setLineDash([5, 5]);
            
            ctx.beginPath();
            ctx.moveTo(lastPoint.x, lastPoint.y);
            ctx.lineTo(root.mousePos.x, root.mousePos.y);
            ctx.stroke();
            
            // Draw crosshair at cursor
            var crossSize = 8;
            ctx.setLineDash([]);
            
            ctx.beginPath();
            ctx.moveTo(root.mousePos.x - crossSize, root.mousePos.y);
            ctx.lineTo(root.mousePos.x + crossSize, root.mousePos.y);
            ctx.moveTo(root.mousePos.x, root.mousePos.y - crossSize);
            ctx.lineTo(root.mousePos.x, root.mousePos.y + crossSize);
            ctx.stroke();
            
            // Show distance preview for distance mode
            if (root.tool.mode === MeasurementTool.Distance) {
                var dist = root.tool.calculateDistance(
                    lastPoint,
                    {"x": root.mousePos.x, "y": root.mousePos.y}
                );
                
                var text = dist.toFixed(2) + " m";
                var textX = (lastPoint.x + root.mousePos.x) / 2;
                var textY = (lastPoint.y + root.mousePos.y) / 2;
                
                ctx.font = root.textSize + "px sans-serif";
                var textWidth = ctx.measureText(text).width;
                var textHeight = root.textSize;
                
                // Draw background
                ctx.fillStyle = "rgba(0, 0, 0, 0.7)";
                ctx.fillRect(
                    textX - textWidth/2 - 2,
                    textY - textHeight/2 - 2,
                    textWidth + 4,
                    textHeight + 4
                );
                
                // Draw text
                ctx.fillStyle = root.textColor;
                ctx.textBaseline = "middle";
                ctx.textAlign = "center";
                ctx.fillText(text, textX, textY);
            }
        }
    }
    
    // Update canvas when tool changes
    Connections {
        target: root.tool
        function onPointsChanged() {
            lineCanvas.requestPaint();
            areaCanvas.requestPaint();
            hoverCanvas.requestPaint();
        }
        
        function onModeChanged() {
            lineCanvas.requestPaint();
            areaCanvas.requestPaint();
            hoverCanvas.requestPaint();
        }
    }
    
    // Update canvas when mouse moves
    onMousePosChanged: {
        hoverCanvas.requestPaint();
    }
    
    // Initial paint
    Component.onCompleted: {
        lineCanvas.requestPaint();
        areaCanvas.requestPaint();
        hoverCanvas.requestPaint();
    }
}
