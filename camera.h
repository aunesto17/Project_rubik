#ifndef CAMERA_H_
#define CAMERA_H_

// camera.h - class to store camera information

#include "helper.h"

class Camera {
public:
    float distance = 10.0f;      // Distance from center
    float rotationAngle = 5.0f;  // Current rotation angle around Y axis
    float elevation = 35.264f;   // Elevation angle (standard isometric is about 35.264 degrees)
    float autoRotateSpeed = 0.5f;// Degrees per frame for auto-rotation (0 to disable)
    bool isAutoRotating = false; // Toggle for auto-rotation
    // Mouse control variables
    bool isDragging = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;


    float getDistance() const { return distance; }
    float getRotationAngle() const { return rotationAngle; }
    float getElevation() const { return elevation; }
    float getAutoRotateSpeed() const { return autoRotateSpeed; }
    bool getIsAutoRotating() const { return isAutoRotating; }
    bool getIsDragging() const { return isDragging; }
    double getLastMouseX() const { return lastMouseX; }
    double getLastMouseY() const { return lastMouseY; }

    void setDistance(float distance) { this->distance = distance; }
    void setRotationAngle(float rotationAngle) { this->rotationAngle = rotationAngle; }
    void setElevation(float elevation) { this->elevation = elevation; }
    void setAutoRotateSpeed(float autoRotateSpeed) { this->autoRotateSpeed = autoRotateSpeed; }
    void setIsAutoRotating(bool isAutoRotating) { this->isAutoRotating = isAutoRotating; }
    void setIsDragging(bool isDragging) { this->isDragging = isDragging; }
    void setLastMouseX(double lastMouseX) { this->lastMouseX = lastMouseX; }
    void setLastMouseY(double lastMouseY) { this->lastMouseY = lastMouseY; }

    //destructor
    ~Camera() {}
};



#endif // CAMERA_H_



