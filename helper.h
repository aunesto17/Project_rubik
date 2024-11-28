#ifndef HELPER_H_
#define HELPER_H_

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <fstream>
#include "stb_image.h"	// libreria para cargar imagenes
#include <queue>
#include "vertex.h"
#include <deque>
#include "camera.h"	
#include "solver/solve.h"
#include "solver/random.h"


// Matrix locations
GLint viewLoc;
GLint projLoc;
GLint modelLoc;
GLint viewLocSB;
GLint projLocSB;

float toRadians(float degrees) {
    return degrees * (M_PI / 180.0f);
}

// Helper function to normalize a vector
void normalize(float v[3]) {
    float length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    v[0] /= length;
    v[1] /= length;
    v[2] /= length;
}

// Helper function for cross product
void cross(float v1[3], float v2[3], float result[3]) {
    result[0] = v1[1] * v2[2] - v1[2] * v2[1];
    result[1] = v1[2] * v2[0] - v1[0] * v2[2];
    result[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void lookAt(float eye[3], float center[3], float up[3], float matrix[16]) {
    // w = eye - center / ||eye - center||
    float w[3] = { 
        eye[0] - center[0],
        eye[1] - center[1],
        eye[2] - center[2] 
    };
    normalize(w);

    // u = b x w / ||b x w||
    float u[3];
    cross(up, w, u);
    normalize(u);

    // v = w × u
    float v[3];
    cross(w, u, v);

    // Create the matrix
    matrix[0] = u[0];
    matrix[1] = u[1];
    matrix[2] = u[2];
    matrix[3] = -u[0] * eye[0] -u[1] * eye[1] -u[2] * eye[2];

    matrix[4] = v[0];
    matrix[5] = v[1];
    matrix[6] = v[2];
    matrix[7] = -v[0] * eye[0] -v[1] * eye[1] -v[2] * eye[2];

    matrix[8] = w[0];
    matrix[9] = w[1];
    matrix[10] = w[2];
    matrix[11] = -w[0] * eye[0] -w[1] * eye[1] -w[2] * eye[2];

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}

// Helper function to calculate perspective projection matrix
void perspective(float fovDegrees, float aspect, float near, float far, float matrix[16]) {
    // Convert FOV to radians
    float fovRadians = fovDegrees * (M_PI / 180.0f);
    
    // Calculate parameters
    float tanHalfFov = tan(fovRadians / 2.0f);
    float f = 1.0f / tanHalfFov;
    float zRange = near - far;
    
    // Fill the matrix (column-major order as OpenGL expects)
    matrix[0] = f / aspect;  // Scale X
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;
    
    matrix[4] = 0.0f;
    matrix[5] = f;          // Scale Y
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;
    
    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = (far + near) / zRange;  // Scale Z
    matrix[11] = -1.0f;
    
    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = (2.0f * far * near) / zRange;  // Translation Z
    matrix[15] = 0.0f;
}

// Complete camera setup function
void setupCamera(float width, float height) {
    // Create projection matrix
    float fov = 45.0f;
    float aspect = width / height;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    float projMatrix[16];
    
    // Calculate perspective projection matrix
    perspective(fov, aspect, nearPlane, farPlane, projMatrix);
    
    // Camera position and orientation
    float eye[3] = {0.0f, 0.0f, 3.0f};
    float center[3] = {0.0f, 0.0f, 0.0f};
    float up[3] = {0.0f, 1.0f, 0.0f};
    float viewMatrix[16];
    
    // Calculate view matrix
    lookAt(eye, center, up, viewMatrix);
    
    
    // Send matrices to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projMatrix);
    
    // Model matrix (identity)
    float modelMatrix[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelMatrix);
}

// Helper function for orthographic projection matrix
void ortho(float left, float right, float bottom, float top, float near, float far, float matrix[16]) {

    matrix[0] = 2.0f / (right - left);
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = -(right + left) / (right - left);

    matrix[4] = 0.0f;
    matrix[5] = 2.0f / (top - bottom);
    matrix[6] = 0.0f;
    matrix[7] = -(top + bottom) / (top - bottom);

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = -2.0f / (far - near);
    matrix[11] = -(far + near) / (far - near);

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}

unsigned int loadTexture(const char* path) {
    // Print current working directory
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    std::cout << "Attempting to load texture: " << path << std::endl;

    // Check if file exists
    std::ifstream f(path);
    if (!f.good()) {
        std::cout << "Error: File does not exist!" << std::endl;
        return 0;
    }

    // Get file size
    f.seekg(0, std::ios::end);
    size_t fileSize = f.tellg();
    f.close();
    std::cout << "File size: " << fileSize << " bytes" << std::endl;

    // Flip textures if needed
    stbi_set_flip_vertically_on_load(true);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);

    // Load image data
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    
    if (data) {
        std::cout << "Image loaded successfully:" << std::endl;
        std::cout << "Width: " << width << std::endl;
        std::cout << "Height: " << height << std::endl;
        std::cout << "Channels: " << nrChannels << std::endl;
        
        // Print first few pixels of the image
            // std::cout << "First 16 bytes of image data:" << std::endl;
            // for(int i = 0; i < 16 && i < width * height * nrChannels; i++) {
            //     std::cout << (int)data[i] << " ";
            //     if((i + 1) % 4 == 0) std::cout << std::endl;
            // }

        GLenum internalFormat;
        GLenum dataFormat;
        if (nrChannels == 1) {
            internalFormat = GL_RED;
            dataFormat = GL_RED;
        }
        else if (nrChannels == 3) {
            internalFormat = GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrChannels == 4) {
            internalFormat = GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        
        // Check for OpenGL errors
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cout << "OpenGL error after texture creation: " << err << std::endl;
        }

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture: " << path << std::endl;
        std::cout << "STB Error: " << stbi_failure_reason() << std::endl;
    }

    stbi_image_free(data);
    return texture;
}


// Also check if the files exist before trying to load them
void checkTextureFiles() {
    const char* textureFiles[] = {
        "letter-u.png",
        "letter-c.png",
        "letter-s.png",
        "letter-p.png",
        "ucsp-logo.png"
    };

    for (const char* file : textureFiles) {
        std::ifstream f(file);
        if (!f.good()) {
            std::cout << "Texture file not found: " << file << std::endl;
        } else {
            std::cout << "Found texture file: " << file << std::endl;
        }
    }
}

void snapToGrid(vec3& vertex, float gridSize = 1.02f) {
    // Round to nearest grid position
    vertex.x = round(vertex.x / gridSize) * gridSize;
    vertex.y = round(vertex.y / gridSize) * gridSize;
    vertex.z = round(vertex.z / gridSize) * gridSize;
}

// Add this helper function
float normalizeAngle(float angle) {
    // Normalize angle to -180 to 180 range
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

glm::vec3 getCubeCenter(const std::vector<vec3>& vertices) {
    glm::vec3 sum(0.0f);
    for (const auto& v : vertices) {
        sum += glm::vec3(v.getX(), v.getY(), v.getZ());
    }
    return sum / static_cast<float>(vertices.size());
}


#endif // HELPER_H_