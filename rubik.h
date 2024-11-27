#ifndef RUBIK_H_
#define RUBIK_H_

// rubik.h - class to store and manipulate the Rubik's cube

#include "figura.h"
#include "helper.h"

#include <iostream>
#include <cstdint>
#include <vector>
using std::vector;
#include <map>
using std::map;
#include <array>
using std::array;
#include <memory>
using std::unique_ptr;
using std::make_unique;
using std::string;


class CuboRubik
{
private:
    typedef unique_ptr<Cubo> cubePtr;

    map<string, cubePtr>            cubeMap;
    map<string, array<string, 9>>   faceMap;
    map<string, array<string, 8>>   sliceMap;

    vector<unsigned> buffers;
    vector<unsigned> vaos;


    // Store VAO and VBO for each cube
    struct CubeBuffers {
        unsigned int VAO;
        unsigned int VBO;
        unsigned int EBO;
    };

    map<string, CubeBuffers> cubeBuffers;

    // animation variables
    struct AnimationState {
        char face;
        float targetAngle;
        float currentAngle;
        float animationSpeed;  // degrees per second
        bool isSlice;
        bool isAnimating;
        bool isClockwise;
        std::vector<std::string> affectedCubes;
        
        AnimationState() : 
            face(' '), 
            targetAngle(0.0f), 
            currentAngle(0.0f),
            animationSpeed(360.0f),  // complete 360° rotation in 1 second
            isSlice(false),
            isAnimating(false),
            isClockwise(false) {}
            
        void reset() {
            face = ' ';
            targetAngle = 0.0f;
            currentAngle = 0.0f;
            isSlice = false;
            isAnimating = false;
            isClockwise = false;
            affectedCubes.clear();
        }
    };

    AnimationState currentAnimation;
    float lastFrameTime;
    const float EPSILON = 0.0001f;

    void setupBuffers() {
        // Texture coordinates for each face
        const vector<vec2> texCoords = {
            vec2(0.0f, 0.0f),   // bottom-left
            vec2(1.0f, 0.0f),   // bottom-right
            vec2(1.0f, 1.0f),   // top-right
            vec2(1.0f, 1.0f),   // top-right
            vec2(0.0f, 1.0f),   // top-left
            vec2(0.0f, 0.0f)    // bottom-left
        };


        for (const auto& cube : cubeMap) {
            CubeBuffers buffers;

            // Create combined vertex data (position, color, texture)
            vector<float> vertexData;
            const auto& vertices = cube.second->vertices;
            const auto& colors = cube.second->getColors();
            const auto& activeFaces = cube.second->getActiveFaces();

            for (size_t face = 0; face < 6; face++) {
                for (size_t v = 0; v < 6; v++) {
                    size_t vertexIndex = face * 6 + v;
                    
                    // Position
                    vertexData.push_back(vertices[vertexIndex].getX());
                    vertexData.push_back(vertices[vertexIndex].getY());
                    vertexData.push_back(vertices[vertexIndex].getZ());
                    
                    // color and texture active faces only
                    if(!activeFaces[face]) {
                        vertexData.push_back(0.2);
                        vertexData.push_back(0.2);
                        vertexData.push_back(0.2);

                        vertexData.push_back(0.0f);
                        vertexData.push_back(0.0f);

                        // store data in cube
                        cube.second->vertexColors.push_back(vec3(0.2, 0.2, 0.2));
                        cube.second->vertexTexCoords.push_back(vec2(0.0f, 0.0f));
                    } else {
                        const vec3& color = colors[face];
                        vertexData.push_back(color.getX());
                        vertexData.push_back(color.getY());
                        vertexData.push_back(color.getZ());

                        // Texture coordinates
                        vertexData.push_back(texCoords[v].getX());
                        vertexData.push_back(texCoords[v].getY());

                        // store data in cube
                        cube.second->vertexColors.push_back(vec3(color.getX(), color.getY(), color.getZ()));
                        cube.second->vertexTexCoords.push_back(vec2(texCoords[v].getX(), texCoords[v].getY()));
                    }
                }
            }

            // if first cube then print vertexData
            if(cube.first == "LUF"){
                for (size_t i = 0; i < vertexData.size(); i++) {
                    std::cout << vertexData[i] << " ";
                    if((i+1) % 8 == 0) std::cout << std::endl;
                }
            }
            
            // Generate and bind VAO
            glGenVertexArrays(1, &buffers.VAO);
            glGenBuffers(1, &buffers.VBO);

            glBindVertexArray(buffers.VAO);

            // Generate and bind VBO
            
            glBindBuffer(GL_ARRAY_BUFFER, buffers.VBO);
            // Buffer vertex data

            glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), &vertexData[0], GL_STATIC_DRAW);
            
            // Position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            
            // Color attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 
                                (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            
            // Texture attribute
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 
                                (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);
            
            cubeBuffers[cube.first] = buffers;
        }
    }

    // Helper method to rotate vertices around an axis
    void rotateVertices(std::vector<vec3>& vertices, const glm::vec3& axis, float angle) {
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
        
        for (vec3& vertex : vertices) {
            glm::vec4 rotated = rotation * glm::vec4(vertex.x, vertex.y, vertex.z, 1.0f);
            vertex.x = rotated.x;
            vertex.y = rotated.y;
            vertex.z = rotated.z;
        }
    }

    // Helper to get rotation matrix for a specific face/slice
    glm::mat4 getRotationMatrix(char face, float angle) {
        glm::vec3 axis;
        switch(face) {
            // faces
            case 'U': case 'D': // Y-axis rotation
                axis = glm::vec3(0.0f, 1.0f, 0.0f);
                if (face == 'D') angle = -angle;
                break;
            case 'L': case 'R': // X-axis rotation
                axis = glm::vec3(1.0f, 0.0f, 0.0f);
                if (face == 'L') angle = -angle;
                break;
            case 'F': case 'B': // Z-axis rotation
                axis = glm::vec3(0.0f, 0.0f, 1.0f);
                if (face == 'B') angle = -angle;
                break;
            case 'V': // Middle vertical slice (like L)
                axis = glm::vec3(1.0f, 0.0f, 0.0f);
                //angle = -angle;
                break;
            case 'H': // Middle horizontal slice (like U)
                axis = glm::vec3(0.0f, 1.0f, 0.0f);
                //angle = -angle;
                break;
             // slices   
            case 'S': // Middle slice (like F)
                axis = glm::vec3(0.0f, 0.0f, 1.0f);
                break;
        }
        return glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
    }

    // Update cube positions after rotation
    void updateCubePositions(const std::vector<string>& affectedCubes, char face, float angle) {
        glm::mat4 rotation = getRotationMatrix(face, angle);
        
        for (const string& cubeName : affectedCubes) {
            auto& cube = cubeMap[cubeName];
            
            // Rotate vertex positions
            for (vec3& vertex : cube->vertices) {
                glm::vec4 rotated = rotation * glm::vec4(vertex.x, vertex.y, vertex.z, 1.0f);
                vertex.x = rotated.x;
                vertex.y = rotated.y;
                vertex.z = rotated.z;
            }
            
            // Update vertex buffers
            glBindBuffer(GL_ARRAY_BUFFER, cubeBuffers[cubeName].VBO);
            
            // Create combined vertex data
            vector<float> vertexData;
            const auto& vertices = cube->vertices;
            const auto& colors = cube->vertexColors;
            const auto& texCoords = cube->vertexTexCoords;
            
            for (size_t i = 0; i < vertices.size(); i++) {
                // Position
                vertexData.push_back(vertices[i].x);
                vertexData.push_back(vertices[i].y);
                vertexData.push_back(vertices[i].z);
                
                // Color
                vertexData.push_back(colors[i].x);
                vertexData.push_back(colors[i].y);
                vertexData.push_back(colors[i].z);
                
                // Texture coordinates
                vertexData.push_back(texCoords[i].x);
                vertexData.push_back(texCoords[i].y);
            }
            
            glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData.size() * sizeof(float), &vertexData[0]);
        }
    }

    // Helper to update face map after rotation
    void updateFaceMapAfterRotation(char face, bool clockwise) {
        auto& faceCubes = faceMap[string(1, face)];
        vector<string> newOrder;
        
        // Different rotation patterns based on face and direction
        if (clockwise) {
            // Clockwise rotation pattern: corners rotate 6->8->2->0->6, edges rotate 7->5->1->3->7
            newOrder = {faceCubes[6], faceCubes[3], faceCubes[0],
                       faceCubes[7], faceCubes[4], faceCubes[1],
                       faceCubes[8], faceCubes[5], faceCubes[2]};
        } else {
            // Counter-clockwise rotation pattern: corners rotate 6->0->2->8->6, edges rotate 7->3->1->5->7
            newOrder = {faceCubes[2], faceCubes[5], faceCubes[8],
                       faceCubes[1], faceCubes[4], faceCubes[7],
                       faceCubes[0], faceCubes[3], faceCubes[6]};
        }
        
        // check if there are no duplicates in a face
        for (size_t i = 0; i < newOrder.size(); i++) {
            for (size_t j = i + 1; j < newOrder.size(); j++) {
                if (newOrder[i] == newOrder[j]) {
                    std::cerr << "\t Error: duplicate cube in face " << face << std::endl;
                    return;
                }
            }
        }
        
        // Update the face map with new positions
        faceCubes = array<string, 9>();
        for (int i = 0; i < 9; i++) {
            faceCubes[i] = newOrder[i];
        }   

        // Update adjacent faces
        updateAdjacentFaces(face, clockwise);
    }

    // helper to update slice map after rotation
    void updateSliceMapAfterRotation(char slice, bool clockwise) {
        auto& sliceCubes = sliceMap[string(1, slice)];
        vector<string> newOrder;
        
        // Different rotation patterns based on slice and direction
        if(slice == 'S'){
            if (clockwise) {
            // Clockwise rotation pattern: corners rotate 6->8->2->0->6, edges rotate 7->5->1->3->7
                newOrder = {sliceCubes[6], sliceCubes[7], sliceCubes[0],
                            sliceCubes[1],                 sliceCubes[2],
                            sliceCubes[3], sliceCubes[4], sliceCubes[5]};
            } else {
            // Counter-clockwise rotation pattern: corners rotate 6->0->2->8->6, edges rotate 7->3->1->5->7
                newOrder = {sliceCubes[2], sliceCubes[3], sliceCubes[4],
                            sliceCubes[5],                 sliceCubes[6], 
                            sliceCubes[7], sliceCubes[0], sliceCubes[1]};
            }
        }
            else if(slice == 'V' || slice == 'H'){
            if (clockwise) {
            // Clockwise rotation pattern: corners rotate 6->8->2->0->6, edges rotate 7->5->1->3->7
                newOrder = {sliceCubes[2], sliceCubes[3], sliceCubes[4],
                            sliceCubes[5],                sliceCubes[6],
                            sliceCubes[7], sliceCubes[0], sliceCubes[1]};
            } else {
            // Counter-clockwise rotation pattern: corners rotate 6->0->2->8->6, edges rotate 7->3->1->5->7
                newOrder = {sliceCubes[6], sliceCubes[7], sliceCubes[0],
                            sliceCubes[1],                sliceCubes[2], 
                            sliceCubes[3], sliceCubes[4], sliceCubes[5]};
            }
        }
        
        
        // check if there are no duplicates in a slice
        for (size_t i = 0; i < newOrder.size(); i++) {
            for (size_t j = i + 1; j < newOrder.size(); j++) {
                if (newOrder[i] == newOrder[j]) {
                    std::cerr << "\t Error: duplicate cube in slice " << slice << std::endl;
                    return;
                }
            }
        }
        
        // Update the slice map with new positions
        sliceCubes = array<string, 8>();
        for (int i = 0; i < 8; i++) {
            sliceCubes[i] = newOrder[i];
        }   

        // Update adjacent faces
        updateAdjacentFaces(slice, clockwise);
    }

    // Helper to update adjacent faces after rotation
    void updateAdjacentFaces(char face, bool clockwise) {
        vector<string> temp;
        switch(face) {
            case 'U':
                if (clockwise) {
                    // Move top to right (U -> R)
                    faceMap["R"][2] = faceMap["U"][2];
                    faceMap["R"][1] = faceMap["U"][5];
                    faceMap["R"][0] = faceMap["U"][8];
                    // Move right to bottom (U -> F)
                    faceMap["F"][0] = faceMap["U"][6];
                    faceMap["F"][1] = faceMap["U"][7];
                    faceMap["F"][2] = faceMap["U"][8];
                    // Move bottom to left (U -> L)
                    faceMap["L"][0] = faceMap["U"][0];
                    faceMap["L"][1] = faceMap["U"][3];
                    faceMap["L"][2] = faceMap["U"][6];
                    // Move top to back (U -> B)
                    faceMap["B"][2] = faceMap["U"][0];
                    faceMap["B"][1] = faceMap["U"][1];
                    faceMap["B"][0] = faceMap["U"][2];
                    // side middle slice
                    sliceMap["S"][0] = faceMap["U"][3];
                    sliceMap["S"][1] = faceMap["U"][4];
                    sliceMap["S"][2] = faceMap["U"][5];
                    // middle vertical slice
                    sliceMap["V"][0] = faceMap["U"][1];
                    sliceMap["V"][1] = faceMap["U"][4];
                    sliceMap["V"][2] = faceMap["U"][7];
                } else {
                    faceMap["R"][2] = faceMap["U"][2];
                    faceMap["R"][1] = faceMap["U"][5];
                    faceMap["R"][0] = faceMap["U"][8];
                    
                    faceMap["F"][0] = faceMap["U"][6];
                    faceMap["F"][1] = faceMap["U"][7];
                    faceMap["F"][2] = faceMap["U"][8];
                    
                    faceMap["L"][0] = faceMap["U"][0];
                    faceMap["L"][1] = faceMap["U"][3];
                    faceMap["L"][2] = faceMap["U"][6];
                    
                    faceMap["B"][2] = faceMap["U"][0];
                    faceMap["B"][1] = faceMap["U"][1];
                    faceMap["B"][0] = faceMap["U"][2];

                    sliceMap["S"][0] = faceMap["U"][3];
                    sliceMap["S"][1] = faceMap["U"][4];
                    sliceMap["S"][2] = faceMap["U"][5];
                    sliceMap["V"][0] = faceMap["U"][1];
                    sliceMap["V"][1] = faceMap["U"][4];
                    sliceMap["V"][2] = faceMap["U"][7];
                }
                break;

            case 'L':
                if (clockwise) {
                    faceMap["F"][0] = faceMap["L"][2];
                    faceMap["F"][3] = faceMap["L"][5];
                    faceMap["F"][6] = faceMap["L"][8];

                    faceMap["D"][0] = faceMap["L"][8];
                    faceMap["D"][3] = faceMap["L"][7];
                    faceMap["D"][6] = faceMap["L"][6];

                    faceMap["B"][2] = faceMap["L"][0];
                    faceMap["B"][5] = faceMap["L"][3];
                    faceMap["B"][8] = faceMap["L"][6];

                    faceMap["U"][0] = faceMap["L"][0];
                    faceMap["U"][3] = faceMap["L"][1];
                    faceMap["U"][6] = faceMap["L"][2];

                    sliceMap["S"][0] = faceMap["L"][1];
                    sliceMap["S"][7] = faceMap["L"][4];
                    sliceMap["S"][6] = faceMap["L"][7];
                    sliceMap["H"][0] = faceMap["L"][3];
                    sliceMap["H"][1] = faceMap["L"][4];
                    sliceMap["H"][2] = faceMap["L"][5];
                    
                } else {
                    faceMap["F"][0] = faceMap["L"][2];
                    faceMap["F"][3] = faceMap["L"][5];
                    faceMap["F"][6] = faceMap["L"][8];

                    faceMap["D"][0] = faceMap["L"][8];
                    faceMap["D"][3] = faceMap["L"][7];
                    faceMap["D"][6] = faceMap["L"][6];

                    faceMap["B"][2] = faceMap["L"][0];
                    faceMap["B"][5] = faceMap["L"][3];
                    faceMap["B"][8] = faceMap["L"][6];

                    faceMap["U"][0] = faceMap["L"][0];
                    faceMap["U"][3] = faceMap["L"][1];
                    faceMap["U"][6] = faceMap["L"][2];

                    sliceMap["S"][0] = faceMap["L"][1];
                    sliceMap["S"][7] = faceMap["L"][4];
                    sliceMap["S"][6] = faceMap["L"][7];
                    sliceMap["H"][0] = faceMap["L"][3];
                    sliceMap["H"][1] = faceMap["L"][4];
                    sliceMap["H"][2] = faceMap["L"][5];
                }
                break;

            case 'F':
                if (clockwise) {
                    // Move top to right (F -> R)
                    faceMap["R"][0] = faceMap["F"][2];
                    faceMap["R"][3] = faceMap["F"][5];
                    faceMap["R"][6] = faceMap["F"][8];
                    // Move right to bottom (F -> D)
                    faceMap["D"][0] = faceMap["F"][6];
                    faceMap["D"][1] = faceMap["F"][7];
                    faceMap["D"][2] = faceMap["F"][8];
                    // Move bottom to left (F -> L)
                    faceMap["L"][2] = faceMap["F"][0];
                    faceMap["L"][5] = faceMap["F"][3];
                    faceMap["L"][8] = faceMap["F"][6];
                    // Move left to top (F -> U)
                    faceMap["U"][6] = faceMap["F"][0];
                    faceMap["U"][7] = faceMap["F"][1];
                    faceMap["U"][8] = faceMap["F"][2];

                    sliceMap["V"][2] = faceMap["F"][1];
                    sliceMap["V"][3] = faceMap["F"][4];
                    sliceMap["V"][4] = faceMap["F"][7];
                    sliceMap["H"][2] = faceMap["F"][3];
                    sliceMap["H"][3] = faceMap["F"][4];
                    sliceMap["H"][4] = faceMap["F"][5];
                } else {
                    // Move top to right (F -> R)
                    faceMap["R"][0] = faceMap["F"][2];
                    faceMap["R"][3] = faceMap["F"][5];
                    faceMap["R"][6] = faceMap["F"][8];
                    // Move right to bottom (F -> D)
                    faceMap["D"][0] = faceMap["F"][6];
                    faceMap["D"][1] = faceMap["F"][7];
                    faceMap["D"][2] = faceMap["F"][8];
                    // Move bottom to left (F -> L)
                    faceMap["L"][2] = faceMap["F"][0];
                    faceMap["L"][5] = faceMap["F"][3];
                    faceMap["L"][8] = faceMap["F"][6];
                    // Move left to top (F -> U)
                    faceMap["U"][6] = faceMap["F"][0];
                    faceMap["U"][7] = faceMap["F"][1];
                    faceMap["U"][8] = faceMap["F"][2];

                    sliceMap["V"][2] = faceMap["F"][1];
                    sliceMap["V"][3] = faceMap["F"][4];
                    sliceMap["V"][4] = faceMap["F"][7];
                    sliceMap["H"][2] = faceMap["F"][3];
                    sliceMap["H"][3] = faceMap["F"][4];
                    sliceMap["H"][4] = faceMap["F"][5];
                }
                break;

            case 'R':
                if (clockwise) {
                    faceMap["B"][0] = faceMap["R"][2];
                    faceMap["B"][3] = faceMap["R"][5];
                    faceMap["B"][6] = faceMap["R"][8];

                    faceMap["D"][2] = faceMap["R"][6];
                    faceMap["D"][5] = faceMap["R"][7];
                    faceMap["D"][8] = faceMap["R"][8];

                    faceMap["F"][2] = faceMap["R"][0];
                    faceMap["F"][5] = faceMap["R"][3];
                    faceMap["F"][8] = faceMap["R"][6];

                    faceMap["U"][2] = faceMap["R"][2];
                    faceMap["U"][5] = faceMap["R"][1];
                    faceMap["U"][8] = faceMap["R"][0];

                    sliceMap["S"][2] = faceMap["R"][1];
                    sliceMap["S"][3] = faceMap["R"][4];
                    sliceMap["S"][4] = faceMap["R"][7];

                    sliceMap["H"][6] = faceMap["R"][5];
                    sliceMap["H"][5] = faceMap["R"][4];
                    sliceMap["H"][4] = faceMap["R"][3];
                } else {
                    faceMap["B"][0] = faceMap["R"][2];
                    faceMap["B"][3] = faceMap["R"][5];
                    faceMap["B"][6] = faceMap["R"][8];

                    faceMap["D"][2] = faceMap["R"][6];
                    faceMap["D"][5] = faceMap["R"][7];
                    faceMap["D"][8] = faceMap["R"][8];

                    faceMap["F"][2] = faceMap["R"][0];
                    faceMap["F"][5] = faceMap["R"][3];
                    faceMap["F"][8] = faceMap["R"][6];

                    faceMap["U"][2] = faceMap["R"][2];
                    faceMap["U"][5] = faceMap["R"][1];
                    faceMap["U"][8] = faceMap["R"][0];

                    sliceMap["S"][2] = faceMap["R"][1];
                    sliceMap["S"][3] = faceMap["R"][4];
                    sliceMap["S"][4] = faceMap["R"][7];
                    sliceMap["H"][6] = faceMap["R"][5];
                    sliceMap["H"][5] = faceMap["R"][4];
                    sliceMap["H"][4] = faceMap["R"][3];
                }
                break;

            case 'B':
                if (clockwise) {
                    faceMap["L"][0] = faceMap["B"][2];
                    faceMap["L"][3] = faceMap["B"][5];
                    faceMap["L"][6] = faceMap["B"][8];

                    faceMap["D"][6] = faceMap["B"][8];
                    faceMap["D"][7] = faceMap["B"][7];
                    faceMap["D"][8] = faceMap["B"][6];

                    faceMap["R"][2] = faceMap["B"][0];
                    faceMap["R"][5] = faceMap["B"][3];
                    faceMap["R"][8] = faceMap["B"][6];
                    
                    faceMap["U"][0] = faceMap["B"][2];
                    faceMap["U"][1] = faceMap["B"][1];
                    faceMap["U"][2] = faceMap["B"][0];

                    sliceMap["V"][0] = faceMap["B"][1];
                    sliceMap["V"][7] = faceMap["B"][4];
                    sliceMap["V"][6] = faceMap["B"][7];
                    sliceMap["H"][0] = faceMap["B"][5];
                    sliceMap["H"][7] = faceMap["B"][4];
                    sliceMap["H"][6] = faceMap["B"][3];
                } else {
                    faceMap["L"][0] = faceMap["B"][2];
                    faceMap["L"][3] = faceMap["B"][5];
                    faceMap["L"][6] = faceMap["B"][8];

                    faceMap["D"][6] = faceMap["B"][8];
                    faceMap["D"][7] = faceMap["B"][7];
                    faceMap["D"][8] = faceMap["B"][6];

                    faceMap["R"][2] = faceMap["B"][0];
                    faceMap["R"][5] = faceMap["B"][3];
                    faceMap["R"][8] = faceMap["B"][6];
                    
                    faceMap["U"][0] = faceMap["B"][2];
                    faceMap["U"][1] = faceMap["B"][1];
                    faceMap["U"][2] = faceMap["B"][0];

                    sliceMap["V"][0] = faceMap["B"][1];
                    sliceMap["V"][7] = faceMap["B"][4];
                    sliceMap["V"][6] = faceMap["B"][7];
                    sliceMap["H"][0] = faceMap["B"][5];
                    sliceMap["H"][7] = faceMap["B"][4];
                    sliceMap["H"][6] = faceMap["B"][3];
                }
                break;
            
            case 'D':
                if (clockwise) {
                    faceMap["R"][6] = faceMap["D"][2];
                    faceMap["R"][7] = faceMap["D"][5];
                    faceMap["R"][8] = faceMap["D"][8];

                    faceMap["B"][6] = faceMap["D"][8];
                    faceMap["B"][7] = faceMap["D"][7];
                    faceMap["B"][8] = faceMap["D"][6];

                    faceMap["L"][6] = faceMap["D"][6];
                    faceMap["L"][7] = faceMap["D"][3];
                    faceMap["L"][8] = faceMap["D"][0];

                    faceMap["F"][6] = faceMap["D"][0];
                    faceMap["F"][7] = faceMap["D"][1];
                    faceMap["F"][8] = faceMap["D"][2];

                    sliceMap["S"][6] = faceMap["D"][3];
                    sliceMap["S"][5] = faceMap["D"][4];
                    sliceMap["S"][4] = faceMap["D"][5];
                    sliceMap["V"][4] = faceMap["D"][1];
                    sliceMap["V"][5] = faceMap["D"][4];
                    sliceMap["V"][6] = faceMap["D"][7];
                } else {
                    faceMap["R"][6] = faceMap["D"][2];
                    faceMap["R"][7] = faceMap["D"][5];
                    faceMap["R"][8] = faceMap["D"][8];

                    faceMap["B"][6] = faceMap["D"][8];
                    faceMap["B"][7] = faceMap["D"][7];
                    faceMap["B"][8] = faceMap["D"][6];

                    faceMap["L"][6] = faceMap["D"][6];
                    faceMap["L"][7] = faceMap["D"][3];
                    faceMap["L"][8] = faceMap["D"][0];

                    faceMap["F"][6] = faceMap["D"][0];
                    faceMap["F"][7] = faceMap["D"][1];
                    faceMap["F"][8] = faceMap["D"][2];

                    sliceMap["S"][6] = faceMap["D"][3];
                    sliceMap["S"][5] = faceMap["D"][4];
                    sliceMap["S"][4] = faceMap["D"][5];
                    sliceMap["V"][4] = faceMap["D"][1];
                    sliceMap["V"][5] = faceMap["D"][4];
                    sliceMap["V"][6] = faceMap["D"][7];
                }
                break;

            case 'S':
                if (clockwise) {
                    faceMap["U"][3] = sliceMap["S"][0];
                    faceMap["U"][4] = sliceMap["S"][1];
                    faceMap["U"][5] = sliceMap["S"][2];
                    faceMap["L"][1] = sliceMap["S"][0];
                    faceMap["L"][4] = sliceMap["S"][7];
                    faceMap["L"][7] = sliceMap["S"][6];
                    faceMap["R"][1] = sliceMap["S"][2];
                    faceMap["R"][4] = sliceMap["S"][3];
                    faceMap["R"][7] = sliceMap["S"][4];
                    faceMap["D"][3] = sliceMap["S"][6];
                    faceMap["D"][4] = sliceMap["S"][5];
                    faceMap["D"][5] = sliceMap["S"][4];

                    sliceMap["V"][1] = sliceMap["S"][1];
                    sliceMap["V"][5] = sliceMap["S"][5];
                    sliceMap["H"][1] = sliceMap["S"][7];
                    sliceMap["H"][5] = sliceMap["S"][3];
                    
                } else {
                    faceMap["U"][3] = sliceMap["S"][0];
                    faceMap["U"][4] = sliceMap["S"][1];
                    faceMap["U"][5] = sliceMap["S"][2];
                    faceMap["L"][1] = sliceMap["S"][0];
                    faceMap["L"][4] = sliceMap["S"][7];
                    faceMap["L"][7] = sliceMap["S"][6];
                    faceMap["R"][1] = sliceMap["S"][2];
                    faceMap["R"][4] = sliceMap["S"][3];
                    faceMap["R"][7] = sliceMap["S"][4];
                    faceMap["D"][3] = sliceMap["S"][6];
                    faceMap["D"][4] = sliceMap["S"][5];
                    faceMap["D"][5] = sliceMap["S"][4];

                    sliceMap["V"][1] = sliceMap["S"][1];
                    sliceMap["V"][5] = sliceMap["S"][5];
                    sliceMap["H"][1] = sliceMap["S"][7];
                    sliceMap["H"][5] = sliceMap["S"][3];
                }
                break;

            case 'V':
                if (clockwise) {
                    faceMap["U"][1] = sliceMap["V"][0];
                    faceMap["U"][4] = sliceMap["V"][1];
                    faceMap["U"][7] = sliceMap["V"][2];
                    faceMap["F"][1] = sliceMap["V"][2];
                    faceMap["F"][4] = sliceMap["V"][3];
                    faceMap["F"][7] = sliceMap["V"][4];
                    faceMap["B"][1] = sliceMap["V"][0];
                    faceMap["B"][4] = sliceMap["V"][7];
                    faceMap["B"][7] = sliceMap["V"][6];
                    faceMap["D"][1] = sliceMap["V"][4];
                    faceMap["D"][4] = sliceMap["V"][5];
                    faceMap["D"][7] = sliceMap["V"][6];

                    sliceMap["S"][1] = sliceMap["V"][1];
                    sliceMap["S"][5] = sliceMap["V"][5];
                    sliceMap["H"][7] = sliceMap["V"][7];
                    sliceMap["H"][3] = sliceMap["V"][3];
                    
                } else {
                    faceMap["U"][1] = sliceMap["V"][0];
                    faceMap["U"][4] = sliceMap["V"][1];
                    faceMap["U"][7] = sliceMap["V"][2];
                    faceMap["F"][1] = sliceMap["V"][2];
                    faceMap["F"][4] = sliceMap["V"][3];
                    faceMap["F"][7] = sliceMap["V"][4];
                    faceMap["B"][1] = sliceMap["V"][0];
                    faceMap["B"][4] = sliceMap["V"][7];
                    faceMap["B"][7] = sliceMap["V"][6];
                    faceMap["D"][1] = sliceMap["V"][4];
                    faceMap["D"][4] = sliceMap["V"][5];
                    faceMap["D"][7] = sliceMap["V"][6];

                    sliceMap["S"][1] = sliceMap["V"][1];
                    sliceMap["S"][5] = sliceMap["V"][5];
                    sliceMap["H"][7] = sliceMap["V"][7];
                    sliceMap["H"][3] = sliceMap["V"][3];
                }
                break;

            case 'H':
                if (clockwise) {
                    faceMap["L"][3] = sliceMap["H"][0];
                    faceMap["L"][4] = sliceMap["H"][1];
                    faceMap["L"][5] = sliceMap["H"][2];
                    faceMap["F"][3] = sliceMap["H"][2];
                    faceMap["F"][4] = sliceMap["H"][3];
                    faceMap["F"][5] = sliceMap["H"][4];
                    faceMap["R"][3] = sliceMap["H"][4];
                    faceMap["R"][4] = sliceMap["H"][5];
                    faceMap["R"][5] = sliceMap["H"][6];
                    faceMap["B"][3] = sliceMap["H"][6];
                    faceMap["B"][4] = sliceMap["H"][7];
                    faceMap["B"][5] = sliceMap["H"][0];

                    sliceMap["S"][7] = sliceMap["H"][1];
                    sliceMap["S"][3] = sliceMap["H"][5];
                    sliceMap["V"][7] = sliceMap["H"][7];
                    sliceMap["V"][3] = sliceMap["H"][3];
                } else {
                    faceMap["L"][3] = sliceMap["H"][0];
                    faceMap["L"][4] = sliceMap["H"][1];
                    faceMap["L"][5] = sliceMap["H"][2];
                    faceMap["F"][3] = sliceMap["H"][2];
                    faceMap["F"][4] = sliceMap["H"][3];
                    faceMap["F"][5] = sliceMap["H"][4];
                    faceMap["R"][3] = sliceMap["H"][4];
                    faceMap["R"][4] = sliceMap["H"][5];
                    faceMap["R"][5] = sliceMap["H"][6];
                    faceMap["B"][3] = sliceMap["H"][6];
                    faceMap["B"][4] = sliceMap["H"][7];
                    faceMap["B"][5] = sliceMap["H"][0];

                    sliceMap["S"][7] = sliceMap["H"][1];
                    sliceMap["S"][3] = sliceMap["H"][5];
                    sliceMap["V"][7] = sliceMap["H"][7];
                    sliceMap["V"][3] = sliceMap["H"][3];
                }
                break;
        }
    }

    // Helper method to debug face maps
    void printFaceMap(char face) {
        const auto& faceCubes = faceMap[string(1, face)];
        std::cout << "Face " << face << " map:" << std::endl;
        for (int i = 0; i < 9; i++) {
            std::cout << faceCubes[i] << " ";
            if ((i + 1) % 3 == 0) std::cout << std::endl;
        }
    }

    // Helper method to debug slice maps
    void printSliceMap(char slice) {
        const auto& sliceCubes = sliceMap[string(1, slice)];
        std::cout << "Slice " << slice << " map:" << std::endl;
        for (int i = 0; i < 8; i++) {
            std::cout << sliceCubes[i] << " ";
        }
        std::cout << std::endl;
    }

    // Method to rotate a face
    void rotateFace(char face, float angle) {
        if (currentAnimation.isAnimating) {
            return; // Don't start new animation while one is in progress
        }

        angle = normalizeAngle(angle);
        // Setup animation
        currentAnimation.face = face;
        currentAnimation.targetAngle = angle;
        currentAnimation.currentAngle = 0.0f;
        currentAnimation.isSlice = false;
        currentAnimation.isAnimating = true;
        currentAnimation.isClockwise = (angle < 0);

        //vector<string> affectedCubes;
        
        // Get affected cubes based on face
        // if (faceMap.find(string(1, face)) != faceMap.end()) {
        //     const auto& faceCubes = faceMap[string(1, face)];
        //     affectedCubes.insert(affectedCubes.end(), faceCubes.begin(), faceCubes.end());
        // }

        // print affectedCubes
        // for (const auto& cube : affectedCubes) {
        //     std::cout << cube << " ";
        // }

        currentAnimation.affectedCubes.clear();
        if (faceMap.find(std::string(1, face)) != faceMap.end()) {
            const auto& faceCubes = faceMap[std::string(1, face)];
            currentAnimation.affectedCubes.insert(
                currentAnimation.affectedCubes.end(), 
                faceCubes.begin(), 
                faceCubes.end()
            );
        }

        //bool clockwise = (angle < 0);
        //updateCubePositions(currentAnimation.affectedCubes, face, angle);
        //updateFaceMapAfterRotation(face, clockwise);
        
        // Debug print to verify face map update
        // std::cout << "Face " << face << " rotated " << (clockwise ? "clockwise" : "counter-clockwise") << std::endl;
        // printFaceMap(face);
        // std::cout << "-------------------" << std::endl;
        // printFaceMap('U');
        // std::cout << "-------------------" << std::endl;
        // printFaceMap('L');
        // std::cout << "-------------------" << std::endl;
        // printFaceMap('F');
        // std::cout << "-------------------" << std::endl;
        // printFaceMap('R');
        // std::cout << "-------------------" << std::endl;
        // printFaceMap('B');
        // std::cout << "-------------------" << std::endl;
        // printFaceMap('D');
        // std::cout << "-------------------" << std::endl;
        
        // printSliceMap('V');
        // std::cout << "-------------------" << std::endl;
        // printSliceMap('H');
        // std::cout << "-------------------" << std::endl;
        // printSliceMap('S');
        // std::cout << "-------------------" << std::endl;
    }

    void rotateSlice(char slice, float angle) {
        if (currentAnimation.isAnimating) {
            return; // Don't start new animation while one is in progress
        }
        
        //vector<string> affectedCubes;
        
        // Get affected cubes based on slice
        // if (sliceMap.find(string(1, slice)) != sliceMap.end()) {
        //     const auto& sliceCubes = sliceMap[string(1, slice)];
        //     affectedCubes.insert(affectedCubes.end(), sliceCubes.begin(), sliceCubes.end());
        // }

        angle = normalizeAngle(angle);

        // Setup animation
        currentAnimation.face = slice;
        currentAnimation.targetAngle = angle;
        currentAnimation.currentAngle = 0.0f;
        currentAnimation.isSlice = true;
        currentAnimation.isAnimating = true;
        currentAnimation.isClockwise = (angle < 0);

        currentAnimation.affectedCubes.clear();
        if (sliceMap.find(std::string(1, slice)) != sliceMap.end()) {
            const auto& sliceCubes = sliceMap[std::string(1, slice)];
            currentAnimation.affectedCubes.insert(
                currentAnimation.affectedCubes.end(), 
                sliceCubes.begin(), 
                sliceCubes.end()
            );
        }
        
        //bool clockwise = (angle < 0);
        //updateCubePositions(currentAnimation.affectedCubes, slice, angle);
        //updateSliceMapAfterRotation(slice, clockwise);

        std::cout << "slice " << slice << " angle: " << angle << std::endl;
        // printSliceMap(slice);
        // std::cout << "-------------------" << std::endl;
        // printFaceMap('F');
        // std::cout << "-------------------" << std::endl;
    }

    // solver methods
    std::vector<char> POSSIBLE_MOVES = {'U', 'L', 'F', 'R', 'B', 'D'};
    std::vector<char> POSSIBLE_SLICES = {'V', 'H', 'S'};

    struct Move {
        char face;
        float angle;
        bool isSlice;
        
        Move(char f, float a, bool slice = false) : face(f), angle(a), isSlice(slice) {}
    };

    std::vector<Move> generateScrambleSequence(int numMoves) {
        std::vector<Move> sequence;
        std::random_device rd;
        std::mt19937 gen(rd());
        
        // Distribution for moves and angles
        std::uniform_int_distribution<> moveDist(0, POSSIBLE_MOVES.size() - 1);
        //std::uniform_int_distribution<> sliceDist(0, POSSIBLE_SLICES.size() - 1);
        std::uniform_int_distribution<> angleDist(0, 1);  // 0: 90°, 1: -90°, 2: 180°
        //std::uniform_int_distribution<> moveTypeDist(0, 4); // 20% chance for slice moves
        
        char lastFace = ' ';
        
        for (int i = 0; i < numMoves; i++) {
            char face;
            //bool isSlice = (moveTypeDist(gen) == 0); // 20% chance for slice moves
            
            // Avoid same face moves consecutively
            do {
                face = POSSIBLE_MOVES[moveDist(gen)];
            } while (face == lastFace);
            
            lastFace = face;
            
            // Determine angle
            float angle;
            int angleType = angleDist(gen);
            switch (angleType) {
                case 0: angle = 90.0f; break;
                case 1: angle = -90.0f; break;
                //case 2: angle = 180.0f; break;
            }
            
            sequence.push_back(Move(face, -90.0f, 0));
        }
    
        return sequence;
    }

    // animation methods
    void updateCubeBuffer(const std::string& cubeName) {
        auto& cube = cubeMap[cubeName];
        glBindBuffer(GL_ARRAY_BUFFER, cubeBuffers[cubeName].VBO);
        
        // Create combined vertex data
        std::vector<float> vertexData;
        const auto& vertices = cube->vertices;
        const auto& colors = cube->vertexColors;
        const auto& texCoords = cube->vertexTexCoords;
        
        for (size_t i = 0; i < vertices.size(); i++) {
            // Position
            vertexData.push_back(vertices[i].x);
            vertexData.push_back(vertices[i].y);
            vertexData.push_back(vertices[i].z);
            
            // Color
            vertexData.push_back(colors[i].x);
            vertexData.push_back(colors[i].y);
            vertexData.push_back(colors[i].z);
            
            // Texture coordinates
            vertexData.push_back(texCoords[i].x);
            vertexData.push_back(texCoords[i].y);
        }
        
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData.size() * sizeof(float), &vertexData[0]);
    }

    void finishAnimation() {
        // Snap all affected cubes to grid
        // for (const std::string& cubeName : currentAnimation.affectedCubes) {
        //     auto& cube = cubeMap[cubeName];
        //     for (vec3& vertex : cube->vertices) {
        //         snapToGrid(vertex);
        //     }
        //     // Update buffer after snapping
        //     updateCubeBuffer(cubeName);
        // }
        // Update cube maps
        bool clockwise = (currentAnimation.targetAngle < 0);
        if (currentAnimation.isSlice) {
            updateSliceMapAfterRotation(currentAnimation.face, clockwise);
        } else {
            updateFaceMapAfterRotation(currentAnimation.face, clockwise);
        }
        
        // Reset animation state
        currentAnimation.reset();
    }
    // Helper method to update positions during animation
    void updateFacePositions(char face, float angleChange) {
        glm::mat4 rotation = getRotationMatrix(face, angleChange);
        
        for (const std::string& cubeName : currentAnimation.affectedCubes) {
            auto& cube = cubeMap[cubeName];
            
            // Rotate vertices
            for (vec3& vertex : cube->vertices) {
                glm::vec4 rotated = rotation * glm::vec4(vertex.x, vertex.y, vertex.z, 1.0f);
                vertex.x = rotated.x;
                vertex.y = rotated.y;
                vertex.z = rotated.z;
            }
            
            // Update buffers
            updateCubeBuffer(cubeName);
        }
    }

    void updateSlicePositions(char slice, float angleChange) {
        glm::mat4 rotation = getRotationMatrix(slice, angleChange);
        
        for (const std::string& cubeName : currentAnimation.affectedCubes) {
            auto& cube = cubeMap[cubeName];
            
            // Rotate vertices
            for (vec3& vertex : cube->vertices) {
                glm::vec4 rotated = rotation * glm::vec4(vertex.x, vertex.y, vertex.z, 1.0f);
                vertex.x = rotated.x;
                vertex.y = rotated.y;
                vertex.z = rotated.z;
            }
            
            // Update buffers
            updateCubeBuffer(cubeName);
        }
    }

public:
    CuboRubik(float lastFrameTime) : lastFrameTime(lastFrameTime) {
        
        std::cout << "Rubik's Cube Constructor" << std::endl;
        //initializeCubes();
        //setupBuffers();
    }

    void init(){
        // Initialize all cubes with their positions
        initializeCubes();
        setupBuffers();
    }

    void initializeCubes() {
        // Create the 26 outer cubes (excluding center)
        const float offset = 1.02f;  // Slight gap between cubes
        
        vector<array<float, 3>> positions = {
            // Left layer
            {-offset, -offset, -offset}, {-offset, -offset, 0.0f}, {-offset, -offset, offset},
            {-offset, 0.0f, -offset}, {-offset, 0.0f, 0.0f}, {-offset, 0.0f, offset},
            {-offset, offset, -offset}, {-offset, offset, 0.0f}, {-offset, offset, offset},
            
            // Middle layer
            {0.0f, -offset, -offset}, {0.0f, -offset, 0.0f}, {0.0f, -offset, offset},
            {0.0f, 0.0f, -offset}, {0.0f, 0.0f, offset},
            {0.0f, offset, -offset}, {0.0f, offset, 0.0f}, {0.0f, offset, offset},
            
            // Right Layer
            {offset, -offset, -offset}, {offset, -offset, 0.0f}, {offset, -offset, offset},
            {offset, 0.0f, -offset}, {offset, 0.0f, 0.0f}, {offset, 0.0f, offset},
            {offset, offset, -offset}, {offset, offset, 0.0f}, {offset, offset, offset}
        };
        
        vector<string> names = {
            "LDB", "LD", "LDF", "LB", "L", "LF", "LUB", "LU", "LUF",
            "DB", "D", "DF", "B", "F", "UB", "U", "UF",
            "RDB", "RD", "RDF", "RB", "R", "RF", "RUB", "RU", "RUF"
        };

        // 0 = TOP, 1 = LEFT, 2 = FRONT, 3 = RIGHT, 4 = BACK, 5 = BOTTOM
        vector<array<bool, 6>> visibleFaces = {
            {0, 1, 0, 0, 1, 1}, {0, 1, 0, 0, 0, 1}, {0, 1, 1, 0, 0, 1},
            {0, 1, 0, 0, 1, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0},
            {1, 1, 0, 0, 1, 0}, {1, 1, 0, 0, 0, 0}, {1, 1, 1, 0, 0, 0},
            
            // Middle layer
            {0, 0, 0, 0, 1, 1}, {0, 0, 0, 0, 0, 1}, {0, 0, 1, 0, 0, 1},
            {0, 0, 0, 0, 1, 0}, {0, 0, 1, 0, 0, 0},
            {1, 0, 0, 0, 1, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 1, 0, 0, 0},
            
            // Right Layer
            {0, 0, 0, 1, 1, 1}, {0, 0, 0, 1, 0, 1}, {0, 0, 1, 1, 0, 1},
            {0, 0, 0, 1, 1, 0}, {0, 0, 0, 1, 0, 0}, {0, 0, 1, 1, 0, 0},
            {1, 0, 0, 1, 1, 0}, {1, 0, 0, 1, 0, 0}, {1, 0, 1, 1, 0, 0}
        };
        
        for (size_t i = 0; i < positions.size(); i++) {
            const auto& pos = positions[i];
            cubeMap[names[i]] = make_unique<Cubo>(
                names[i], 1.0f, 
                vec3(pos[0], pos[1], pos[2]),
                visibleFaces[i]
            );
        }

        // Initialize the faces.
        this->faceMap["U"] = {{"LUB", "UB", "RUB", "LU", "U", "RU", "LUF", "UF", "RUF"}};
        this->faceMap["L"] = {{"LUB", "LU", "LUF", "LB", "L", "LF", "LDB", "LD", "LDF"}};
        this->faceMap["F"] = {{"LUF", "UF", "RUF", "LF", "F", "RF", "LDF", "DF", "RDF"}};
        this->faceMap["R"] = {{"RUF", "RU", "RUB", "RF", "R", "RB", "RDF", "RD", "RDB"}};
        this->faceMap["B"] = {{"RUB", "UB", "LUB", "RB", "B", "LB", "RDB", "DB", "LDB"}};
        this->faceMap["D"] = {{"LDF", "DF", "RDF", "LD", "D", "RD", "LDB", "DB", "RDB"}};

        // Initialize the middle slices.
        this->sliceMap["V"] = {{"UB", "U", "UF", "F", "DF", "D", "DB", "B"}};
        this->sliceMap["H"] = {{"LB", "L", "LF", "F", "RF", "R", "RB", "B"}};
        this->sliceMap["S"] = {{"LU", "U", "RU", "R", "RD", "D", "LD", "L"}}; // view from front
    }

    void draw(unsigned int shaderProgram) {
        int faceIndexLoc = glGetUniformLocation(shaderProgram, "faceIndex");
        if(faceIndexLoc == -1) {
            std::cout << "Warning: Could not find faceIndex uniform location" << std::endl;
        }

        // for (const auto& cube : cubeBuffers) {
        //     glBindVertexArray(cube.second.VAO);
            
        //     //glDrawArrays(GL_TRIANGLES, 0, 36);  // 6 vertices per face * 6 faces

        //     // Draw each face with appropriate texture
        //     for(int face = 0; face < 6; face++) {
        //         glUniform1i(faceIndexLoc, face);
        //         glDrawArrays(GL_TRIANGLES, 0, 36);
        //     }
        // }

        for (const auto& cube : cubeBuffers) {
            glBindVertexArray(cube.second.VAO);
        
            // Draw each face with its corresponding texture
            for (int face = 0; face < 6; face++) {
                // Set which face we're drawing (this tells the shader which texture to use)
                glUniform1i(faceIndexLoc, face);
                
                // Draw just this face (6 vertices)
                glDrawArrays(GL_TRIANGLES, face * 6, 6);
                
                // Check for errors
                GLenum err = glGetError();
                if (err != GL_NO_ERROR) {
                    std::cout << "OpenGL error in draw: " << err 
                            << " (face " << face << ")" << std::endl;
                }
            }
        }

        // draw only first cube
        // glBindVertexArray(cubeBuffers["LDB"].VAO);
        // for (int face = 0; face < 6; face++) {
        //         // Set which face we're drawing (this tells the shader which texture to use)
        //         glUniform1i(faceIndexLoc, face);
                
        //         // Draw just this face (6 vertices)
        //         glDrawArrays(GL_TRIANGLES, face * 6, 6);
                
        //         // Check for errors
        //         GLenum err = glGetError();
        //         if (err != GL_NO_ERROR) {
        //             std::cout << "OpenGL error in draw: " << err 
        //                     << " (face " << face << ")" << std::endl;
        //         }
        //     }

        // glBindVertexArray(cubeBuffers["LDF"].VAO);
        // for (int face = 0; face < 6; face++) {
        //         // Set which face we're drawing (this tells the shader which texture to use)
        //         glUniform1i(faceIndexLoc, face);
                
        //         // Draw just this face (6 vertices)
        //         glDrawArrays(GL_TRIANGLES, face * 6, 6);
                
        //         // Check for errors
        //         GLenum err = glGetError();
        //         if (err != GL_NO_ERROR) {
        //             std::cout << "OpenGL error in draw: " << err 
        //                     << " (face " << face << ")" << std::endl;
        //         }
        // }
    }

    // Update animation state - call this every frame
    void updateAnimation(float currentTime) {
        if (!currentAnimation.isAnimating) {
            return;
        }

        //lastFrameTime = glfwGetTime();
        float sign = (currentAnimation.isClockwise) ? -1.0f : 1.0f;

        float deltaTime = currentTime - lastFrameTime;
        deltaTime = std::min(deltaTime, 0.016f);
        std::cout << "deltaTime: " << deltaTime << std::endl;
        //lastFrameTime = currentTime;
        

        // Calculate new angle
        float angleChange = sign * currentAnimation.animationSpeed * deltaTime;
        //float angleChange = 5.0f;
        std::cout << "angleChange: " << angleChange << std::endl;
        std::cout << "current angle: " << currentAnimation.currentAngle << std::endl;
        std::cout << "target angle: " << currentAnimation.targetAngle << std::endl;
        float remainingAngle = currentAnimation.targetAngle - currentAnimation.currentAngle;

        std::cout << "Angle change: "<< angleChange <<"\t Remaining angle: " << remainingAngle << std::endl;	
        
        if (std::abs(remainingAngle) <= abs(angleChange) + EPSILON) {
            // Animation complete
            angleChange = remainingAngle;
            updateCubePositions(currentAnimation.affectedCubes, currentAnimation.face, angleChange);
            finishAnimation();
            return;
        }

        // Update positions for this frame
        // if (currentAnimation.isSlice) {
        //     updateSlicePositions(currentAnimation.face, angleChange);
        // } else {
        //     updateFacePositions(currentAnimation.face, angleChange);
        // }
        updateCubePositions(currentAnimation.affectedCubes, currentAnimation.face, angleChange);

        currentAnimation.currentAngle += angleChange;
        lastFrameTime = currentTime;
    }

    void rotateU() {
        rotateFace('U', -90.0f);
    }
    void rotateUPrime() {
        rotateFace('U', 90.0f);
    }
    void rotateU2() {
        rotateU();
        rotateU();
    }

    void rotateL() {
        rotateFace('L', -90.0f);
    }
    void rotateLPrime() {
        rotateFace('L', 90.0f);
    }
    void rotateL2() {
        rotateL();
        rotateL();
    }
    
    void rotateF() {
        rotateFace('F', -90.0f);
    }
    void rotateFPrime() {
        rotateFace('F', 90.0f);
    }
    void rotateF2() {
        rotateF();
        rotateF();
    }

    void rotateR() {
        rotateFace('R', -90.0f);
    }
    void rotateRPrime() {
        rotateFace('R', 90.0f);
    }
    void rotateR2() {
        rotateR();
        rotateR();
    }

    void rotateB() {
        rotateFace('B', -90.0f);
    }
    void rotateBPrime() {
        rotateFace('B', 90.0f);
    }
    void rotateB2() {
        rotateB();
        rotateB();
    }

    void rotateD() {
        rotateFace('D', -90.0f);
    }
    void rotateDPrime() {
        rotateFace('D', 90.0f);
    }
    void rotateD2() {
        rotateD();
        rotateD();
    }

    void rotateSV() {
        rotateSlice('V', -90.0f);
    }

    void rotateSH() {
        rotateSlice('H', -90.0f);
    }
    void rotateSS() {
        rotateSlice('S', -90.0f);
    }

    std::vector<string> scrambleCube(int numMoves) {
        std::vector<std::string> sequenceString;
        std::vector<Move> scrambleSequence = generateScrambleSequence(numMoves);
        
        std::cout << "Executing scramble sequence:" << scrambleSequence.size() <<std::endl;
        for (const Move& move : scrambleSequence) {
            std::cout << move.face << "(" << move.angle << "°) ";
            // if (move.isSlice) {
            //     rotateSlice(move.face, move.angle);
            // } else {
            //     rotateFace(move.face, move.angle);
            // }
            if(move.angle < 0)
            {
                switch(move.face) {
                    case 'U':
                        rotateU();
                        sequenceString.push_back("U");
                        break;
                    case 'L':
                        rotateL();
                        sequenceString.push_back("L");
                        break;
                    case 'F':
                        rotateF();
                        sequenceString.push_back("F");
                        break;
                    case 'R':
                        rotateR();
                        sequenceString.push_back("R");
                        break;
                    case 'B':
                        rotateB();
                        sequenceString.push_back("B");
                        break;
                    case 'D':
                        rotateD();
                        sequenceString.push_back("D");
                        break;
                    case 'V':
                        rotateSV();
                        break;
                    case 'H':
                        rotateSH();
                        break;
                    case 'S':
                        rotateSS();
                        break;
                }
            }
            else{
                switch(move.face) {
                    case 'U':
                        rotateUPrime();
                        sequenceString.push_back("U'");
                        break;
                    case 'L':
                        rotateLPrime();
                        sequenceString.push_back("L'");
                        break;
                    case 'F':
                        rotateFPrime();
                        sequenceString.push_back("F'");
                        break;
                    case 'R':
                        rotateRPrime();
                        sequenceString.push_back("R'");
                        break;
                    case 'B':
                        rotateBPrime();
                        sequenceString.push_back("B'");
                        break;
                    case 'D':
                        rotateDPrime();
                        sequenceString.push_back("D'");
                        break;
                    case 'V':
                        rotateSV();
                        break;
                    case 'H':
                        rotateSH();
                        break;
                    case 'S':
                        rotateSS();
                        break;
                }
            }
            
        }
        std::cout << std::endl;
        return sequenceString;
    }

    void moveFromList(std::vector<std::string> str)
    {
        std::string mov = "";	
        while(!(str.empty())){
            mov = str[0];
            str.erase(str.begin());
            if(mov == "U")
            {
                rotateU();
            }
            else if(mov == "U'")
            {
                rotateUPrime();
            }
            else if(mov == "U2")
            {
                rotateU2();
            }
            else if(mov == "L")
            {
                rotateL();
            }
            else if(mov == "L'")
            {
                rotateLPrime();
            }
            else if(mov == "L2")
            {
                rotateL2();
            }
            else if(mov == "F")
            {
                rotateF();
            }
            else if(mov == "F'")
            {
                rotateFPrime();
            }
            else if(mov == "F2")
            {
                rotateF2();
            }
            else if(mov == "R")
            {
                rotateR();
            }
            else if(mov == "R'")
            {
                rotateRPrime();
            }
            else if(mov == "R2")
            {
                rotateR2();
            }
            else if(mov == "B")
            {
                rotateB();
            }
            else if(mov == "B'")
            {
                rotateBPrime();
            }
            else if(mov == "B2")
            {
                rotateB2();
            }
            else if(mov == "D")
            {
                rotateD();
            }
            else if(mov == "D'")
            {
                rotateDPrime();
            }
            else if(mov == "D2")
            {
                rotateD2();
            }
        }
    }

    void setLastFrameTime(float time) {
        lastFrameTime = time;
    }


    ~CuboRubik() {
        // Cleanup buffers
        for (const auto& cube : cubeBuffers) {
            glDeleteVertexArrays(1, &cube.second.VAO);
            glDeleteBuffers(1, &cube.second.VBO);
        }
    }

    
};

#endif // RUBIK_H_