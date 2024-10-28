#ifndef MATRIZ_H_
#define MATRIZ_H_

#include <vector>
#include <array>
#include "vertex.h"

class matriz4x4
{
public:
    std::array<float, 16> mat{
        1, 0, 0, 0, 
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    // matriz vacia 
    matriz4x4(){
        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 4; j++){
                mat[i * 4 + j] = 0.0f;
            }
        }
    }

    // multiplicar dos matrices
    matriz4x4 & multMat( const matriz4x4 &mat2 ){
        std::array<float, 16> res;
        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 4; j++){
                res[i * 4 + j] = 0.0f;
                for(int k = 0; k < 4; k++){
                    res[i * 4 + j] += mat[i * 4 + k] * mat2.mat[k * 4 + j];
                }
            }
        }
        mat = res;
        return *this;
    }

    // multiplicacion de matrices con vector
    std::vector<float> multFig(std::vector<float> & v){
        std::vector<float> res;
        //std::cout << "Figsize: " << v.size() << std::endl;
        for(int i=0; i < v.size(); i+=3){
            float x = v[i];
            float y = v[i+1];
            float z = v[i+2];
            float w = 1.0f;
            float x_res = x*mat[0 * 4 + 0] + y*mat[0 * 4 + 1] + z*mat[0 * 4 + 2] + w*mat[0 * 4 + 3];
            float y_res = x*mat[1 * 4 + 0] + y*mat[1 * 4 + 1] + z*mat[1 * 4 + 2] + w*mat[1 * 4 + 3];
            float z_res = x*mat[2 * 4 + 0] + y*mat[2 * 4 + 1] + z*mat[2 * 4 + 2] + w*mat[2 * 4 + 3];
            res.push_back(x_res);
            res.push_back(y_res);
            res.push_back(z_res);
        }
        return res;
    }


    ~matriz4x4() {};
};



#endif // MATRIZ_H_