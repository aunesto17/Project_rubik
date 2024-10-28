#ifndef FIGURA_H_
#define FIGURA_H_

#include <vector>
#include <iostream>
#include <math.h>

class Figura
{  
public:
    std::vector<float> vertices, verticesOrig;
    std::vector<unsigned int> indices;

    Figura(){};
    
    void addVertex(float x, float y, float z);
    void addIndex(unsigned int index);
    const std::vector<float> & getVertices() const {
        return vertices;
    }
    const std::vector<unsigned int> & getIndices() const{
        return indices;
    }

    size_t getVerticesSize(){
        return vertices.size();
    }

    ssize_t getIndicesSize(){
        return indices.size();
    }

    void updateFig(std::vector<float> v){
        vertices = v;
    }

    void resetFig(){
        vertices = verticesOrig;
    }

    void clearVertices();
    void clearIndices();

    ~Figura(){};
};

class Casa: public Figura
{
public:
    unsigned int indicesTri[21] = {  // note that we start from 0!
        0, 1, 2,  // top triangle
        3, 4, 12,   // right sqr
        4, 5, 12,   // right sqr
        12, 6, 11, // middle sqr
        6, 7, 11, // middle sqr
        11, 8, 10, // left sqr
        8, 9, 10  // left sqr
    };

    Casa(){
        // coordenadas de la casa completa
        vertices = {
                // triangle
                -0.5f,  0.0f, 0.0f,  // triangle left
                0.0f,  0.75f, 0.0f,  // triangle top
                0.5f, 0.0f, 0.0f,  // triangle right
                // right sqr
                0.375f,  0.0f, 0.0f,  // top right
                0.375f, -0.75f, 0.0f,  // bottom right
                0.125f, -0.75f, 0.0f,  // bottom left
                // middle sqr
                0.125f, -0.5f, 0.0f,  // bottom right
                -0.125f, -0.5f, 0.0f,  // bottom left

                // left sqr
                -0.125f, -0.75f, 0.0f,  // bottom right
                -0.375f, -0.75f, 0.0f,  // bottom left
                -0.375f,  0.0f, 0.0f,  // top left

                // hidden points
                -0.125f,  0.0f, 0.0f,  // top left mid sqr
                0.125f,  0.0f, 0.0f,  // top right mid sqr
            };
        
        indices = {  // note that we start from 0!
                0, 1, 2,  // first Triangle
                3, 4, 5,   // right sqr
                6, 7, 8,   // middle sqr
                9, 10  // left sqr
            };
        
        verticesOrig = vertices;
        // std::cout << "indices size:" << std::endl;
        // std::cout << indices.size() << std::endl;
        // std::cout << "sizeof indicesTri:" << std::endl;
        // std::cout << sizeof(indicesTri) << std::endl;	
    }

    const std::vector<unsigned int> & getIndicesTri() const{
        //return indicesTri;
    }

    size_t getIndicesTriSize(){
        //return indicesTri.size();
    }

    ~Casa() {};
};

class Estrella: public Figura
{
public:
    Estrella(){
        const float steps = 5;
        const float angle = 3.1415926 * 2.f;

        // vertices interiores de la estrella
        float xPosI = 0; float yPosI = 0.25f; float radiusI = 0.15f;
        std::vector<float> vertInt;
	    for(int i = 0; i < steps; i++){
            vertInt.push_back(radiusI * cos(((angle * i)/5) + (3.1415926 / 2.f) + (2 * 3.1415926 / 10.f)));
            vertInt.push_back(radiusI * sin(((angle * i)/5) + (3.1415926 / 2.f) + (2 * 3.1415926 / 10.f)));
            vertInt.push_back(0.0f);
	    }

        // exteriores
	    float xPosE = 0; float yPosE = 0; float radiusE = 0.3f;
	    std::vector<float> vertExt;
	    for(int i = 0; i < steps; i++){
            vertExt.push_back(radiusE * cos(((angle * i)/5) + (3.1415926 / 2.f)));
            vertExt.push_back(radiusE * sin(((angle * i)/5) + (3.1415926 / 2.f)));
            vertExt.push_back(0.0f);
	    }
        double j=0.0f;
        for(int i=0; i<13; i+=3){
            vertices.push_back(vertExt[i]);
            vertices.push_back(vertExt[i+1]);
            vertices.push_back(vertExt[i+2]);
            // colors
            if(j == 0){
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                j+=1.0f;
            } else if (j==1){
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);
                j+=1.0f;
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                j=0.0f;
            } 
            // texture coords
            vertices.push_back(vertExt[i]);
            vertices.push_back(vertExt[i+1]);

            vertices.push_back(vertInt[i]);
            vertices.push_back(vertInt[i+1]);
            vertices.push_back(vertInt[i+2]);
            // colors
            if(j == 0){
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                j+=1.0f;
            } else if (j == 1){
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);
                j+=1.0f;
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                j=0.0f;
            }
            // texture coords
            vertices.push_back(vertInt[i]);
            vertices.push_back(vertInt[i+1]);
        }

        std::cout << "vertices size:" << vertices.size() << std::endl; 
        std::cout << "vertices:" << std::endl;
        for(int i=0; i<vertices.size(); i+=3){
            std::cout << vertices[i] << " " << vertices[i+1] << " " << vertices[i+2] << std::endl;
        }

        indices = { 
            0,1,2,3,4,5,6,7,8,9  
        };

        verticesOrig = vertices;
        
    }

     ~Estrella() {};
};

class Pizza : public Figura {
public:
    float centerX, centerY;
    float centerZ = 0.0f;

    Pizza(float x, float y) : centerX(x), centerY(y) {
        const float circleRadius = 0.25f;
        int segments = 8;  // Increase or decrease for smoother or more jagged edges
        int slices = 4;     // Number of pizza slices

        // Center of the pizza
        vertices.push_back(centerX);
        vertices.push_back(centerY);
        vertices.push_back(centerZ);

        // Generate vertices for the circle's circumference
        for (int i = 0; i <= segments; ++i) {
            float angle = 2.0f * M_PI * i / segments;
            float x = centerX + circleRadius * cos(angle);
            float y = centerY + circleRadius * sin(angle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0.0f);

            if (i > 0) {
                indices.push_back(0);   // Connect to center
                indices.push_back(i);   // Current vertex
                indices.push_back(i + 1); // Next vertex (to form triangles)
            }
        }

        // Add lines to represent pizza slices
        for (int i = 0; i < slices; ++i) {
            float angle = 2.0f * M_PI * i / slices;
            float x = centerX + circleRadius * cos(angle);
            float y = centerY + circleRadius * sin(angle);

            // Add the vertex for the slice line endpoint
            vertices.push_back(x);  // Black color for slice lines
            vertices.push_back(y);
            vertices.push_back(0.0f);
            indices.push_back(0); // Connect center to the slice line endpoint
            indices.push_back(vertices.size() - 1);
        }

        // std::cout << "vertices size:" << vertices.size() << std::endl;
        // std::cout << "indices size:" << indices.size() << std::endl;
        // for(int i=0; i<vertices.size(); i+=3){
        //     std::cout << vertices[i] << " " << vertices[i+1] << " " << vertices[i+2] << std::endl;
        // }
        // for(int i=0; i<indices.size(); i++){
        //     std::cout << indices[i] << std::endl;        
        // }

        verticesOrig = vertices;
    }
    // const std::vector<Vertex>& getVertices() const {
    //     return vertices;
    // }
};

class Cubo : public Figura
{
public:
    Cubo(){
        vertices = {
            //back (-z) green
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
            // front (+z) blue
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
            // left (-x) red
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
            // right (+x) orange
            0.5f,  0.5f,  0.5f,  1.0f, 0.5f, 0.0f,  0.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 0.5f, 0.0f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.5f, 0.0f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.5f, 0.0f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.5f, 0.0f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.5f, 0.0f,  0.0f, 0.0f,
            // bottom (-y) yellow
            -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
            0.5f, -0.5f, 0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
            0.5f, -0.5f, 0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, 0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f,
            // top (+y) white
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f
        };
        indices = {
            // front
            0, 1, 2,
            2, 3, 0,
            // right
            1, 5, 6,
            6, 2, 1,
            // back
            7, 6, 5,
            5, 4, 7,
            // left
            4, 0, 3,
            3, 7, 4,
            // bottom
            4, 5, 1,
            1, 0, 4,
            // top
            3, 2, 6,
            6, 7, 3
        };

        verticesOrig = vertices;
    }
};


#endif // FIGURA_H_