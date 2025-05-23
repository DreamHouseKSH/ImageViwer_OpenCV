pragma Singleton
import QtQuick 2.15

/**
 * MeasurementTool QML Type
 * 
 * Provides measurement functionality for distance and area in the image viewer.
 * 
 * This is a singleton type that provides the core measurement functionality
 * and can be used from any QML file by importing the module.
 * 
 * Example usage:
 * 
 * import AirPhoto.Viewer.Measurement 1.0
 * 
 * MeasurementTool {
 *     id: measurementTool
 *     pixelScale: 0.1 // 1 pixel = 0.1 meters
 * }
 */
QtObject {
    id: root
    
    /**
     * The current measurement mode
     * 
     * Possible values:
     * - MeasurementTool.None: No measurement active
     * - MeasurementTool.Distance: Measure distance between points
     * - MeasurementTool.Area: Measure area of a polygon
     */
    property int mode: MeasurementTool.None
    
    /**
     * Scale factor from pixels to real-world units (e.g., meters)
     * 
     * For example, a value of 0.1 means 1 pixel = 0.1 meters
     */
    property real pixelScale: 1.0
    
    /**
     * List of measurement points in image coordinates
     */
    property var points: []
    
    /**
     * The calculated distance in real-world units
     * 
     * Only valid when mode is Distance and there are at least 2 points
     */
    readonly property real distance: {
        if (mode !== MeasurementTool.Distance || points.length < 2) {
            return 0.0;
        }
        
        var total = 0.0;
        for (var i = 1; i < points.length; ++i) {
            var dx = points[i].x - points[i-1].x;
            var dy = points[i].y - points[i-1].y;
            total += Math.sqrt(dx*dx + dy*dy);
        }
        return total * pixelScale;
    }
    
    /**
     * The calculated area in real-world units squared
     * 
     * Only valid when mode is Area and there are at least 3 points
     */
    readonly property real area: {
        if (mode !== MeasurementTool.Area || points.length < 3) {
            return 0.0;
        }
        
        var a = 0.0;
        var n = points.length;
        
        for (var i = 0; i < n; ++i) {
            var j = (i + 1) % n;
            a += points[i].x * points[j].y - points[j].x * points[i].y;
        }
        
        return Math.abs(a) * 0.5 * pixelScale * pixelScale;
    }
    
    /**
     * Add a point to the current measurement
     * 
     * @param point The point to add (with x and y properties)
     */
    function addPoint(point) {
        if (mode === MeasurementTool.None) {
            console.warn("MeasurementTool: Cannot add point - no measurement mode set");
            return;
        }
        
        points = points.concat([{"x": point.x, "y": point.y}]);
        pointsChanged();
    }
    
    /**
     * Remove the last point from the current measurement
     */
    function removeLastPoint() {
        if (points.length > 0) {
            points = points.slice(0, -1);
            pointsChanged();
        }
    }
    
    /**
     * Clear all measurement points
     */
    function clear() {
        if (points.length > 0) {
            points = [];
            pointsChanged();
        }
    }
    
    /**
     * Calculate the distance between two points
     * 
     * @param p1 First point with x and y properties
     * @param p2 Second point with x and y properties
     * @return Distance in real-world units
     */
    function calculateDistance(p1, p2) {
        var dx = p2.x - p1.x;
        var dy = p2.y - p1.y;
        return Math.sqrt(dx*dx + dy*dy) * pixelScale;
    }
    
    // Enums
    readonly property int None: 0
    readonly property int Distance: 1
    readonly property int Area: 2
}
