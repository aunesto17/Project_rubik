#ifndef CAMERA_H_
#define CAMERA_H_
// Add these to camera.h

class Camera {
private:
    // Camera attributes
    float distance = 10.0f;  // Distance from target
    float pitch = 30.0f;     // Up/down rotation
    float yaw = 45.0f;       // Left/right rotation
    float fov = 45.0f;       // Field of view
    
    // Camera movement speed
    float moveSpeed = 2.0f;
    float rotateSpeed = 100.0f;
    float zoomSpeed = 2.0f;

    // Minimum and maximum values
    const float MIN_DISTANCE = 5.0f;
    const float MAX_DISTANCE = 20.0f;
    const float MIN_FOV = 1.0f;
    const float MAX_FOV = 90.0f;

public: 
    Camera(float initialDistance = 10.0f, 
           float initialPitch = 30.0f, 
           float initialYaw = 45.0f, 
           float initialFov = 45.0f)
        : distance(initialDistance)
        , pitch(initialPitch)
        , yaw(initialYaw)
        , fov(initialFov)
        , moveSpeed(2000.0f)
        , rotateSpeed(4000.0f)
        , zoomSpeed(500.0f)
    {
    }
    
    // Getters
    float getDistance() const { return distance; }
    float getPitch() const { return pitch; }
    float getYaw() const { return yaw; }
    float getFov() const { return fov; }

    // Setters
    void setDistance(float d) { 
        distance = glm::clamp(d, MIN_DISTANCE, MAX_DISTANCE); 
    }
    void setPitch(float p) { 
        pitch = glm::clamp(p, -89.0f, 89.0f); 
    }
    void setYaw(float y) { yaw = y; }
    void setFov(float f) { 
        fov = glm::clamp(f, MIN_FOV, MAX_FOV); 
    }
    
    // Get the view matrix for the camera
    void getViewMatrix(float* viewMatrix) {
        // Calculate camera position based on spherical coordinates
        float x = distance * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        float y = distance * sin(glm::radians(pitch));
        float z = distance * cos(glm::radians(pitch)) * sin(glm::radians(yaw));

        // Camera position
        glm::vec3 pos(x, y, z);
        // Target (looking at origin)
        glm::vec3 target(0.0f, 0.0f, 0.0f);
        // Up vector
        glm::vec3 up(0.0f, 1.0f, 0.0f);

        // Calculate view matrix
        glm::mat4 view = glm::lookAt(pos, target, up);
        
        // Copy to output matrix
        memcpy(viewMatrix, glm::value_ptr(view), 16 * sizeof(float));
    }

    // Get perspective projection matrix
    void getPerspectiveMatrix(float aspectRatio, float* projMatrix) {
        glm::mat4 proj = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
        memcpy(projMatrix, glm::value_ptr(proj), 16 * sizeof(float));
    }

    // Camera controls
    void moveForward(float deltaTime) {
        //pitch += moveSpeed * deltaTime * 100.0f;
        pitch += moveSpeed * deltaTime;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
    }

    void moveBackward(float deltaTime) {
        //pitch -= moveSpeed * deltaTime * 100.0f;
        pitch -= moveSpeed * deltaTime;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
    }

    void moveLeft(float deltaTime) {
        //yaw -= moveSpeed * deltaTime * 100.0f;
        yaw -= moveSpeed * deltaTime;
        // Keep yaw in range [0, 360)
        if (yaw < 0.0f) {
            yaw += 360.0f;
        }
    }

    void moveRight(float deltaTime) {
        //yaw += moveSpeed * deltaTime * 100.0f;
        yaw += moveSpeed * deltaTime;
        // Keep yaw in range [0, 360)
        if (yaw >= 360.0f) {
            yaw -= 360.0f;
        }
    }

    void zoomIn(float deltaTime) {
        //distance -= zoomSpeed * deltaTime * 5.0f;
        distance -= zoomSpeed * deltaTime;
        distance = glm::clamp(distance, MIN_DISTANCE, MAX_DISTANCE);
    }

    void zoomOut(float deltaTime) {
        //distance += zoomSpeed * deltaTime * 5.0f;
        distance += zoomSpeed * deltaTime;
        distance = glm::clamp(distance, MIN_DISTANCE, MAX_DISTANCE);
    }

    // Adjust movement speeds
    void setMoveSpeed(float speed) { moveSpeed = speed; }
    void setRotateSpeed(float speed) { rotateSpeed = speed; }
    void setZoomSpeed(float speed) { zoomSpeed = speed; }

    // Reset camera to default position
    void reset() {
        distance = 10.0f;
        pitch = 30.0f;
        yaw = 45.0f;
        fov = 45.0f;
    }
};

#endif // CAMERA_H_