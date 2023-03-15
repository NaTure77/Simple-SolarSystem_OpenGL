#ifndef SPHERE_H
#define SPHERE_H

#include <GL/glew.h>
class Sphere
{
public:
    Sphere();
    ~Sphere();
    void init(GLuint vertexPositionID, GLuint texCoord);
    void cleanup();
    void draw();

private:
    int sectorCount, stackCount;
    bool isInited;
    float radius;
    int numsToDraw;
    GLuint m_vao, m_vboVertex, m_vboIndex;

};

#endif // SPHERE_H