#ifndef FIGURA_H_
#define FIGURA_H_

// figura.h - classes to store and change the cube

#include <vector>
#include <iostream>
#include <math.h>
#include "vertex.h"

class Triangle
{
private:
    std::vector<vec3> vertices;
    vec3 normal;
    vec3 tangent;
public:
    Triangle(){ this->vertices.resize(3); }
    Triangle(vec3 v1, vec3 v2, vec3 v3){ setVertices(v1, v2, v3); }
    void setVertices(vec3 v1, vec3 v2, vec3 v3){
        this->vertices.at(0) = v1;
        this->vertices.at(1) = v2;
        this->vertices.at(2) = v3;
    }
    void computeNormal() { 
        // TODOs
    }
    vec3 getNormal() { return this->normal; }
    vec3 getTangent() { 
        // TODO 
    }
    ~Triangle(){};
};

class Objeto
{ 
private:
    std::string name;
    unsigned vao;
public:
    std::vector<vec3> vertices;
    std::vector<vec3> vertexColors;
    std::vector<vec2> vertexTexCoords;

    std::vector<vec3> vertexNormals;
    std::vector<vec3> vertexTangents;
    std::vector<vec3> defaultColors = {
        vec3(1.0f, 1.0f, 1.0f), // UP (W).
        vec3(0.0f, 155.0f/255.0f, 72.0f/255.0f), // LEFT (G).
        vec3(183.0f/255.0f, 18.0f/255.0f, 52.0f/255.0f), // FRONT (R).
        vec3(0.0f, 70.0f/255.0f, 173.0f/255.0f), // RIGHT (B).
        vec3(1.0f, 88.0f/255.0f, 0.0f), // BACK (O).
        vec3(1.0f, 213.0f/255.0f, 0.0f)  // DOWN (Y).
    };
    
    virtual void setColors(std::vector<vec3> & colors) { this->defaultColors = colors; }
    virtual void computeVertexNormals(){
        // TODO
    }

    Objeto(const std::string & name){
        this->name = name;
    }

    std::string getName() { return this->name; }
    unsigned getVAO() { return this->vao; }
    virtual std::vector<vec3> getColors() const { return this->defaultColors; } 
    
    ~Objeto(){};
};


class Cubo : public Objeto
{
private:
    float size;
    std::array<bool, 6> activefaces;

    void buildRect(const vec3 & topLeft,
                   const vec3 & topRight,
                   const vec3 & bottomRight,
                   const vec3 & bottomLeft,
                   std::vector<vec3> & vertBuffer){	
        vertBuffer.push_back(bottomLeft);
        vertBuffer.push_back(bottomRight);
        vertBuffer.push_back(topRight);

        vertBuffer.push_back(topRight);
        vertBuffer.push_back(topLeft);
        vertBuffer.push_back(bottomLeft);
    }

public:
    Cubo(const std::string & name, float size,
        vec3 position, std::array<bool, 6> faces) : Objeto(name){
            const unsigned numVertsPerFace = 6;
            const unsigned numFaces = 6;
            float dist = size / 2.0f;
            std::vector<vec3> normals;

            this->activefaces = faces;

            this->size = size;
            
            // creamos las 6 caras
            // top
            // TL, TR, BR, BL 
            this->buildRect(vec3(-dist, dist, -dist),
                            vec3(dist, dist, -dist),
                            vec3(dist, dist, dist),
                            vec3(-dist, dist, dist),
                            this->vertices);
            // Left.
            this->buildRect(vec3(-dist,  dist, -dist),
                            vec3(-dist,  dist, dist),
                            vec3(-dist, -dist, dist),
                            vec3(-dist, -dist, -dist),
                            this->vertices);

            // Front.
            // TL, TR, BR, BL
            this->buildRect(vec3(-dist,  dist, dist),
                            vec3( dist,  dist, dist),
                            vec3( dist, -dist, dist),
                            vec3(-dist, -dist, dist),
                            this->vertices);

            // Right.
            this->buildRect(vec3(dist,  dist, dist),
                            vec3(dist,  dist, -dist),
                            vec3(dist, -dist, -dist),
                            vec3(dist, -dist, dist),
                            this->vertices);

            // Back.
            // TL, TR, BR, BL
            this->buildRect(vec3( dist,  dist, -dist),
                            vec3(-dist,  dist, -dist), 
                            vec3(-dist, -dist, -dist),
                            vec3( dist, -dist, -dist),
                            this->vertices);

            // Down.
            this->buildRect(vec3(-dist, -dist,  dist),
                            vec3( dist, -dist,  dist),
                            vec3( dist, -dist, -dist),
                            vec3(-dist, -dist, -dist),
                            this->vertices);

            // translate the vertices to position
            for(std::vector<vec3>::iterator vertex = this->vertices.begin();
                vertex != this->vertices.end(); ++vertex){
                    vertex->setX(vertex->getX() + position.getX());
                    vertex->setY(vertex->getY() + position.getY());
                    vertex->setZ(vertex->getZ() + position.getZ());
            }

            //std::cout << "Cubo::Cubo() - vertices.size() = " << this->vertices.size() << std::endl;


            /*
             TODO: crear las normales de los vertices
            */

            /*
             TODO: crear y bindear los vertex buffers,
                    color buffers,
                    y normal buffers
            */

            
        }

    // return active faces 
    virtual std::array<bool, 6> getActiveFaces() const { return this->activefaces; }
    // destructor
    ~Cubo(){};
};


#endif // FIGURA_H_