#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include "helper.h"


//#define degToRad(angleInDegrees) ((angleInDegrees) * M_PI / 180.0)
//#define radToDeg(angleInRadians) ((angleInRadians) * 180.0 / M_PI)

// class Transform
// {
// public:
//     matriz4x4 m;

//     std::stack<float> rotXpila;
//     std::stack<float> rotYpila;
//     std::stack<float> rotZpila;

//     std::stack<vec3> traslacionPila;
//     std::stack<vec3> escalaPila;

//     const float * dataPtr( void ) const { return m.mat.data(); }

//     Transform(){
       
//     }

//     Transform & mult( const matriz4x4 &mat ){
//         m.multMat(mat);
//     }

//     void traslacion( const vec3 & pos, Figura & fig ){
//         std::vector<float> res;
//         traslacionPila.push(pos);

//         m.mat = {
//             1, 0, 0, pos.getX(),
//             0, 1, 0, pos.getY(),
//             0, 0, 1, pos.getZ(),
//             0, 0, 0, 1
//         };

//         fig.updateFig(m.multFig(fig.vertices));
//     }

//     void traslacion_i( Figura & fig ){
//         std::vector<float> res;
//         if(traslacionPila.empty()) 
//         {
//             std::cout << "No hay traslaciones en la pila" << std::endl;
//             return;
//         }
//         vec3 pos = traslacionPila.top();
        
//         m.mat = {
//             1, 0, 0, -pos.getX(),
//             0, 1, 0, -pos.getY(),
//             0, 0, 1, -pos.getZ(),
//             0, 0, 0, 1
//         };

//         fig.updateFig(m.multFig(fig.vertices));
//         traslacionPila.pop();
//     }

//     void escala( const vec3 & esc, Figura & fig ){
//         std::vector<float> res;
//         escalaPila.push(esc);
//         m.mat = {
//             esc.getX()  , 0         , 0         , 0,
//             0           , esc.getY(), 0         , 0,
//             0           , 0         , esc.getZ(), 0,
//             0           , 0         , 0         , 1
//         };

//         fig.updateFig(m.multFig(fig.vertices));
//     }

//     void escala_i( Figura & fig ){
//         std::vector<float> res;
//         if(escalaPila.empty()) 
//         {
//             std::cout << "No hay escalas en la pila" << std::endl;
//             return;
//         }
//         vec3 esc = escalaPila.top();
//         m.mat = {
//             1/esc.getX(), 0           , 0           , 0,
//             0           , 1/esc.getY(), 0           , 0,
//             0           , 0           , 1/esc.getZ(), 0,
//             0           , 0           , 0           , 1
//         };
//         fig.updateFig(m.multFig(fig.vertices));
//         escalaPila.pop();
//     }

//     void rotacionX( float angle, Figura & fig )
//     {
//         std::vector<float> res;
//         float angleRad = degToRad(angle);
//         float c = cos(angleRad);
//         float s = sin(angleRad);
//         rotXpila.push(angleRad);

//         m.mat = {
//             1, 0, 0, 0,
//             0, c, -s, 0,
//             0, s, c, 0,
//             0, 0, 0, 1
//         };

//         fig.updateFig(m.multFig(fig.vertices));
//     }

//     void rotacionX_i( Figura & fig )
//     {
//         if(rotXpila.empty()) 
//         {
//             std::cout << "No hay rotaciones(X) en la pila" << std::endl;
//             return;
//         }
//         std::vector<float> res;
//         float angleRad = rotXpila.top();
//         float c = cos(angleRad);
//         float s = sin(angleRad);

//         m.mat = {
//             1, 0, 0, 0,
//             0, c, s, 0,
//             0, -s, c, 0,
//             0, 0, 0, 1
//         };

//         rotXpila.pop();
//         fig.updateFig(m.multFig(fig.vertices));
//     }

    
//     void rotacionY( float angle, Figura & fig ){
//         std::vector<float> res;
//         float angleRad = degToRad(angle);
//         float c = cos(angleRad);
//         float s = sin(angleRad);
//         rotYpila.push(angleRad);

//         m.mat = {
//             c, 0, s, 0,
//             0, 1, 0, 0,
//             -s, 0, c, 0,
//             0, 0, 0, 1
//         };

//         fig.updateFig(m.multFig(fig.vertices));
//     }

//     void rotacionY_i( Figura & fig ){
//         if(rotYpila.empty()) 
//         {
//             std::cout << "No hay rotaciones(Y) en la pila" << std::endl;
//             return;
//         }
//         std::vector<float> res;
//         float angleRad = rotYpila.top();
//         float c = cos(angleRad);
//         float s = sin(angleRad);

//         m.mat = {
//             c, 0, -s, 0,
//             0, 1, 0, 0,
//             s, 0, c, 0,
//             0, 0, 0, 1
//         };

//         rotYpila.pop();
//         fig.updateFig(m.multFig(fig.vertices));
//     }

//     void rotacionZ( float angle, Figura & fig ){
//         std::vector<float> res;
//         float angleRad = degToRad(angle);
//         float c = cos(angleRad);
//         float s = sin(angleRad);
//         rotZpila.push(angleRad);

//         m.mat = {
//             c, -s, 0, 0,
//             s, c, 0, 0,
//             0, 0, 1, 0,
//             0, 0, 0, 1
//         };

//         fig.updateFig(m.multFig(fig.vertices));
//     }

//     void rotacionZ_i( Figura & fig ){
//         if(rotZpila.empty()) 
//         {
//             std::cout << "No hay rotaciones(Z) en la pila" << std::endl;
//             return;
//         }
//         std::vector<float> res;
//         float angleRad = rotZpila.top();
//         float c = cos(angleRad);
//         float s = sin(angleRad);

//         m.mat = {
//             c, s, 0, 0,
//             -s, c, 0, 0,
//             0, 0, 1, 0,
//             0, 0, 0, 1
//         };

//         rotZpila.pop();
//         fig.updateFig(m.multFig(fig.vertices));
//     }   

//     ~Transform() {}
// };


class Transform {
public:
    glm::mat4 matrix;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    
    Transform() : 
        matrix(1.0f),
        position(0.0f),
        rotation(0.0f),
        scale(1.0f) {}
    
    void updateMatrix() {
        matrix = glm::mat4(1.0f);
        matrix = glm::translate(matrix, position);
        matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        matrix = glm::scale(matrix, scale);
    }
    
    float* getMatrixPtr() {
        return glm::value_ptr(matrix);
    }
};


#endif // TRANSFORM_H_