#ifndef VERTEX_H_
#define VERTEX_H_

#include <vector>

class vec3
{
public:
    float x, y, z;
    vec3(float x = 0.0f, 
            float y = 0.0f, 
            float z = 0.0f) : x(x), y(y), z(z) {}

    std::vector<float> operator+(std::vector<float> & v)
    {
        std::vector<float> res;
        for(int i = 0; i < v.size(); i+=3)
        {
            res.push_back(v[i] + x);
            res.push_back(v[i+1] + y);
            res.push_back(v[i+2] + z);
        }
        return res;
    }

    float getX() const { return x; }
    float getY() const { return y; }
    float getZ() const { return z; }

    void setX( float num ) { this->x = num; }
    void setY( float num ) { this->y = num; }
    void setZ( float num ) { this->z = num; }

    ~vec3() {}
};



#endif // VERTEX_H_