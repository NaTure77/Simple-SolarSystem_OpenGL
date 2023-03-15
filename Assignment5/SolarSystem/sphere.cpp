#include "sphere.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define BUFFER_OFFSET( offset ) ((GLvoid*) (offset))
using namespace glm;
Sphere::Sphere()
{
    isInited = false;
    m_vao = 0;
    m_vboVertex = 0;
    m_vboIndex = 0;

    sectorCount = 40;
    stackCount = 40;
    radius = 1;
    numsToDraw = 0;
}

Sphere::~Sphere()
{

}

#define PI glm::pi<double>()
void Sphere::init(GLuint vertexPositionID, GLuint texCoordID)
{
    //http://www.songho.ca/opengl/gl_sphere.html
    //https://gist.github.com/zwzmzd/0195733fa1210346b00d

    std::vector<vec3> vertices;
    std::vector<vec2> texCoords;
    for (int i = 0; i <= stackCount; i++) {
        double stackAngle = PI * (-0.5 + (double)(i) / stackCount);
        double xy = radius * cos(stackAngle);
        double z = radius * sin(stackAngle);

        for (int j = 0; j <= sectorCount; j++) {
            double sectorAngle = 2 * PI * (double)(j) / sectorCount;
            double x = cos(sectorAngle);
            double y = sin(sectorAngle);
            vertices.push_back(vec3(x * xy, -z , y * xy));

            // vertex tex coord (s, t) range between [0, 1]
            float s = 1 - (float)j / sectorCount;
            float t = (float)i / stackCount;
            texCoords.push_back(vec2(s, t));
        }
    }

    // k1--k1+1
    // |  / |
    // | /  |
    // k2--k2+1
    std::vector<vec3> vertices2;
    std::vector<vec2> texCoords2;
    int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            if (i != 0)
            {
                vertices2.push_back(vertices[k1]);
                vertices2.push_back(vertices[k2]);
                vertices2.push_back(vertices[k1 + 1]);

                texCoords2.push_back(texCoords[k1]);
                texCoords2.push_back(texCoords[k2]);
                texCoords2.push_back(texCoords[k1 + 1]);
            }
            if (i != (stackCount - 1))
            {
                vertices2.push_back(vertices[k1 + 1]);
                vertices2.push_back(vertices[k2]);
                vertices2.push_back(vertices[k2 + 1]);

                texCoords2.push_back(texCoords[k1 + 1]);
                texCoords2.push_back(texCoords[k2]);
                texCoords2.push_back(texCoords[k2 + 1]);
            }
        }
    }

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vboVertex);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboVertex);
    glBufferData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(vec3) + texCoords2.size() * sizeof(vec2), NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices2.size() * sizeof(vec3), &vertices2[0]);
    glBufferSubData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(vec3), texCoords2.size() * sizeof(vec2), &texCoords2[0]);
    glVertexAttribPointer(vertexPositionID, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(vertexPositionID);

    glVertexAttribPointer(texCoordID, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vertices2.size() * sizeof(vec3)));
    glEnableVertexAttribArray(texCoordID);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    

    numsToDraw = vertices2.size();
    isInited = true;
}

void Sphere::cleanup()
{
    if (!isInited) {
        return;
    }
    if (m_vboVertex) {
        glDeleteBuffers(1, &m_vboVertex);
    }
    if (m_vboIndex) {
        glDeleteBuffers(1, &m_vboIndex);
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
    }

    isInited = false;
    m_vao = 0;
    m_vboVertex = 0;
    m_vboIndex = 0;
}

void Sphere::draw()
{
    // draw sphere
    glBindVertexArray(m_vao);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboIndex);
    glDrawArrays(GL_TRIANGLES, 0, numsToDraw);
}

