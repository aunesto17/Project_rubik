#ifndef RUBIK_H_
#define RUBIK_H_

// rubik.h - class to store and manipulate the Rubik's cube

#include "figura.h"
#include "helper.h"
#include "transform.h"

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

    map<string, Transform> cubeTransforms;

    // Current rotation state
    struct RotationState {
        bool isRotating = false;
        float currentAngle = 0.0f;
        float targetAngle = 0.0f;
        char face = ' ';
        bool clockwise = true;
        vector<string> affectedCubes;
    } currentRotation;

    void generateVertexArrayObject(){
        unsigned vao;
        glGenVertexArrays(1, &vao);
        vaos.push_back(vao);
    }

    // Store VAO and VBO for each cube
    struct CubeBuffers {
        unsigned int VAO;
        unsigned int VBO;
        unsigned int EBO;
    };
    
    map<string, CubeBuffers> cubeBuffers;

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

        // const std::vector<std::vector<vec2>> faceTexCoords = {
        //     // Top face
        //     {
        //         vec2(0.0f, 0.0f),  // bottom-left
        //         vec2(1.0f, 0.0f),  // bottom-right
        //         vec2(0.0f, 1.0f),  // top-left
        //         vec2(0.0f, 1.0f),  // top-left
        //         vec2(1.0f, 0.0f),  // bottom-right
        //         vec2(1.0f, 1.0f)   // top-right
        //     },
        //     // Left face
        //     {
        //         vec2(0.0f, 1.0f),  // top-left
        //         vec2(1.0f, 1.0f),  // top-right
        //         vec2(0.0f, 0.0f),  // bottom-left
        //         vec2(0.0f, 0.0f),  // bottom-left
        //         vec2(1.0f, 0.0f),  // bottom-right
        //         vec2(1.0f, 1.0f)   // top-right
        //     },
        //     // Front face
        //     {
        //         vec2(0.0f, 1.0f),
        //         vec2(1.0f, 1.0f),
        //         vec2(0.0f, 0.0f),
        //         vec2(0.0f, 0.0f),
        //         vec2(1.0f, 0.0f),
        //         vec2(1.0f, 1.0f)
        //     },
        //     // Right face
        //     {
        //         vec2(0.0f, 1.0f),
        //         vec2(1.0f, 1.0f),
        //         vec2(0.0f, 0.0f),
        //         vec2(0.0f, 0.0f),
        //         vec2(1.0f, 0.0f),
        //         vec2(1.0f, 1.0f)
        //     },
        //     // Back face
        //     {
        //         vec2(0.0f, 1.0f),
        //         vec2(1.0f, 1.0f),
        //         vec2(0.0f, 0.0f),
        //         vec2(0.0f, 0.0f),
        //         vec2(1.0f, 0.0f),
        //         vec2(1.0f, 1.0f)
        //     },
        //     // Bottom face
        //     {
        //         vec2(0.0f, 1.0f),
        //         vec2(1.0f, 1.0f),
        //         vec2(0.0f, 0.0f),
        //         vec2(0.0f, 0.0f),
        //         vec2(1.0f, 0.0f),
        //         vec2(1.0f, 1.0f)
        //     }
        // };

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
                        vertexData.push_back(0.1f);
                        vertexData.push_back(0.1f);
                        vertexData.push_back(0.1f);

                        vertexData.push_back(0.0f);
                        vertexData.push_back(0.0f);
                    } else {
                        const vec3& color = colors[face];
                        vertexData.push_back(color.getX());
                        vertexData.push_back(color.getY());
                        vertexData.push_back(color.getZ());

                        // Texture coordinates
                        vertexData.push_back(texCoords[v].getX());
                        vertexData.push_back(texCoords[v].getY());
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

public:
    enum class FACE : uint8_t {UP, LEFT, FRONT, RIGHT, BACK, DOWN };
    enum class COLOR  : uint8_t {WHITE, GREEN, RED, BLUE, ORANGE, YELLOW};
    enum class EDGE   : uint8_t {UB, UR, UF, UL, FR, FL, BL, BR, DF, DL, DB, DR};
    enum class CORNER : uint8_t {ULB, URB, URF, ULF, DLF, DLB, DRB, DRF};
    enum class MOVE   : uint8_t
    {
      L, LPRIME, L2,
      R, RPRIME, R2,
      U, UPRIME, U2,
      D, DPRIME, D2,
      F, FPRIME, F2,
      B, BPRIME, B2,
      Y, YPRIME, Y2,
      X, XPRIME, X2,
      Z, ZPRIME, Z2,
      M, MPRIME, M2,
      E, EPRIME, E2,
      S, SPRIME, S2
    };
    CuboRubik(){
        // Initialize all cubes with their positions
        initializeCubes();
        setupBuffers();

        // print the first cube vertices and colors
        // cubeMap["LDB"]->printVertices();
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

            // Left layer
            // {-offset, -offset, offset}, {-offset, -offset, 0.0f}, {-offset, -offset, -offset},
            // {-offset, 0.0f, offset}, {-offset, 0.0f, 0.0f}, {-offset, 0.0f, -offset},
            // {-offset, offset, offset}, {-offset, offset, 0.0f}, {-offset, offset, -offset},
            
            // // Middle layer
            // {0.0f, -offset, offset}, {0.0f, -offset, 0.0f}, {0.0f, -offset, -offset},
            // {0.0f, 0.0f, offset}, {0.0f, 0.0f, -offset},
            // {0.0f, offset, offset}, {0.0f, offset, 0.0f}, {0.0f, offset, -offset},
            
            // // Right Layer
            // {offset, -offset, offset}, {offset, -offset, 0.0f}, {offset, -offset, -offset},
            // {offset, 0.0f, offset}, {offset, 0.0f, 0.0f}, {offset, 0.0f, -offset},
            // {offset, offset, offset}, {offset, offset, 0.0f}, {offset, offset, -offset}
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

        // Initialize the slices.
        this->sliceMap["M"] = {{"UB", "U", "UF", "F", "DF", "D", "DB", "B"}};
        this->sliceMap["E"] = {{"LB", "L", "LF", "F", "RF", "R", "RB", "B"}};
        this->sliceMap["S"] = {{"LU", "U", "RU", "R", "RD", "D", "LD", "L"}};
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
    

    ~CuboRubik() {
        // Cleanup buffers
        for (const auto& cube : cubeBuffers) {
            glDeleteVertexArrays(1, &cube.second.VAO);
            glDeleteBuffers(1, &cube.second.VBO);
        }
    }
};

#endif // RUBIK_H_