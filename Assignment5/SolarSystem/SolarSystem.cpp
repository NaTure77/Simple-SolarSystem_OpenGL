#include "sphere.h"
#include <iostream>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

//https://gist.github.com/zwzmzd/0195733fa1210346b00d
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace glm;
#pragma warning(disable:4996)
#define VIEW_SIZE_X 1280
#define VIEW_SIZE_Y 720
#define BUFFER_OFFSET( offset )    ((GLvoid*) (offset))

Sphere sphere;
GLuint m_sphereProgramID;
GLuint rquatID;
GLuint ModelMatrixID;
GLuint DistanceMatrixID;

GLuint textureID;
GLuint BG_texID;
GLuint sun_texID;
GLuint earth_texID;
GLuint moon_texID;
GLuint mars_texID;
GLuint shading_switchID;
GLuint texture_switchID;
float sphereAngle = 0;

/**************trackball 구현***************/
const GLfloat  Deg2Rad = 3.14159265f / 180.0f;
bool trackingMouse = false;
vec4 rquat = vec4(1.0, 0.0, 0.0, 0.0);
vec3 _axis = vec3(0, 0, 0);
GLfloat _angle = 0;

vec4 multq(vec4 a, vec4 b)
{
    vec3 ayzw = vec3(a.y, a.z, a.w);
    vec3 byzw = vec3(b.y, b.z, b.w);
    return vec4(a.x * b.x - dot(ayzw, byzw), a.x * byzw + b.x * ayzw + cross(byzw, ayzw));
}

vec4 invq(vec4 a)
{
    vec3 ayzw = vec3(a.y, a.z, a.w);
    return (vec4(a.x, -ayzw) / dot(a, a));
}

vec3 lastPos = vec3(0.0, 0.0, 0.0);

vec3 trackball_ptov(int x, int y, int width, int height)
{
    float d, a;
    vec3 v;

    v[0] = (2.0 * x - width) / width;
    v[1] = (height - 2.0 * y) / height;

    d = sqrt(v[0] * v[0] + v[1] * v[1]);
    if (d < 1.0) v[2] = -cos((90 * Deg2Rad) * d);
    else
    {
        v[2] = 0.0;
    }
    //normalize
    v *= 1.0 / sqrt(dot(v, v));
    return v;
}

void mouseMotion(int x, int y)
{
    vec3 curPos;
    vec3 d;
    //반구 위의 한 점.
    curPos = trackball_ptov(x, y, VIEW_SIZE_X, VIEW_SIZE_Y);

    //마우스 누르고 드래그중인 상태일 때.
    d = curPos - lastPos;
    float a = dot(d, d);
    //check if mouse moved
    if (a)
    {
        float speed = 1.1;
        _angle = speed * (90.0 * Deg2Rad) * sqrt(a); // 회전 각도 구하기

        _axis = cross(lastPos, curPos); // 회전축 구하기.

        _axis *= 1.0 / sqrt(dot(_axis, _axis)); // 여기 고침.
        lastPos = curPos;

        rquat = multq(vec4(cos(_angle / 2.0), sin(_angle / 2.0) * _axis.x,
            sin(_angle / 2.0) * _axis.y, sin(-_angle / 2.0) * _axis.z), rquat);
        trackingMouse = true;
    }
    else trackingMouse = false;
}
void startMotion(int x, int y)
{
    // 반구 위의 한 점.
    lastPos = trackball_ptov(x, y, VIEW_SIZE_X, VIEW_SIZE_Y);
}
void stopMotion(int x, int y)
{
    trackingMouse = false;
    _angle = 0.0f;
}

bool lineMode = false;
void mouse(int button, int state, int x, int y)
{

    if (button == GLUT_RIGHT_BUTTON)
    {
        lineMode = !lineMode;
    }

    switch (state)
    {
        case GLUT_DOWN:
            //y = winHeight - y;  // 여기 고침. 이거 있으면 안됨.
            startMotion(x, y);
            break;
        case GLUT_UP:
            stopMotion(x, y);
            break;
    }
}
/******************************************************/
void Tick(int value)
{
    if (lineMode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    glUniform1f(texture_switchID, !lineMode);
    glutPostRedisplay();
    glutTimerFunc(20, Tick, 1);
}

static char* readShaderSource(const char* shaderFile)
{
    FILE* fp = fopen(shaderFile, "r");

    if (fp == NULL)
    {
        fprintf(stderr, "fopen error for %s\n", shaderFile);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buf = new char[size + 1];
    for (int i = 0; i <= size; i++) buf[i] = '\0';
    fread(buf, 1, size, fp);
    fclose(fp);
    buf[size] = '\0';
    return buf;
}
GLuint LoadShaders(const char* vShaderFile, const char* fShaderFile)
{
    //셰이더 코드 불러오기
    GLchar* vSource = readShaderSource(vShaderFile);
    GLchar* fSource = readShaderSource(fShaderFile);

    //셰이더 오브젝트 생성
    GLuint vertObj = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragObj = glCreateShader(GL_FRAGMENT_SHADER);

    //셰이더소스를 오브젝트에 할당
    glShaderSource(vertObj, 1, &vSource, NULL);
    glShaderSource(fragObj, 1, &fSource, NULL);

    //셰이더 컴파일
    glCompileShader(vertObj);
    glCompileShader(fragObj);

    //프로그램 오브젝트 생성
    GLuint myProgObj = glCreateProgram();

    //셰이더 오브젝트를 프로그램에 추가
    glAttachShader(myProgObj, vertObj);
    glAttachShader(myProgObj, fragObj);
    glLinkProgram(myProgObj);

    glDeleteShader(vertObj);
    glDeleteShader(fragObj);

    return myProgObj;
}
mat4 distanceMat_view;
mat4 rotMat_earth_local;
mat4 rotMat_mars_local;
mat4 rotMat_moon_local;
mat4 rotMat_sun_local;
mat4 rotMat_earth;
mat4 rotMat_moon;
mat4 rotMat_mars;

mat4 movMat_earth;
mat4 movMat_moon;
mat4 movMat_mars;

mat4 scaleMat_earth;
mat4 scaleMat_moon;
mat4 scaleMat_mars;
mat4 scaleMat_sun;

float viewScaleValue = 0.5;
void wheel(int wheel, int direction, int x, int y)
{
    viewScaleValue += 0.05 * direction;
    if (viewScaleValue < 0.2)
    {
        viewScaleValue = 0.2;
        return;
    }
    if (viewScaleValue > 1)
    {
        viewScaleValue = 1;
        return;
    }
    distanceMat_view = translate(distanceMat_view, vec3(0,0,5 * direction));
    glUniformMatrix4fv(DistanceMatrixID, 1, GL_FALSE, value_ptr(distanceMat_view));
}

//https://www.solarsystemscope.com/textures/
GLuint LoadTexture(const char* filepath, int id)
{
    glUniform1i(textureID, id);
    GLuint t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filepath, &width, &height, &nrChannels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    stbi_image_free(data);

    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D, t);
    return t;
}

void init()
{
    // Load shaders, link program for drawing sphere
    m_sphereProgramID = LoadShaders("shader/sphereShader.vert", "shader/sphereShader.frag");
    GLuint vPositionID = 0;
    GLuint texCoordID = 1;
    glBindAttribLocation(m_sphereProgramID, vPositionID, "vPosition");
    glBindAttribLocation(m_sphereProgramID, texCoordID, "vTexCoord");
    glUseProgram(m_sphereProgramID);
    sphere.init(vPositionID, texCoordID);

    // Matrix
    mat4 identityMat = mat4(1);
    distanceMat_view = identityMat;
    distanceMat_view = translate(distanceMat_view, vec3(0, 0, -45));

    rotMat_earth_local = rotMat_moon_local = rotMat_sun_local = rotMat_mars_local = 
    rotMat_earth= rotMat_moon = rotMat_mars = identityMat;

    rotMat_earth_local = rotate(rotMat_earth_local, (float)(Deg2Rad * 23.5), vec3(0, 0, 1));
    rotMat_moon_local = rotate(rotMat_moon_local, (float)(Deg2Rad * 6.68), vec3(0, 0, 1));
    rotMat_mars_local = rotate(rotMat_mars_local, (float)(Deg2Rad * 25.19), vec3(0, 0, 1));

    rotMat_moon = rotate(rotMat_moon, (float)(Deg2Rad * 5.1), vec3(0, 0, 1));
    rotMat_mars = rotate(rotMat_mars, (float)(Deg2Rad * 5.65), vec3(0, 0, 1));
    rotMat_mars = rotate(rotMat_mars, (float)(Deg2Rad * 120), vec3(0, 1, 0));

    scaleMat_earth = scaleMat_moon = scaleMat_sun = scaleMat_mars = identityMat;
    scaleMat_sun = scale(scaleMat_sun, vec3(2));
    scaleMat_moon = scale(scaleMat_moon, vec3(0.3));
    scaleMat_earth = scale(scaleMat_earth, vec3(1)); 
    scaleMat_mars = scale(scaleMat_mars, vec3(0.7));

    movMat_earth = movMat_moon = movMat_mars = identityMat;
    movMat_earth = translate(movMat_earth, vec3(10, 0, 0));
    movMat_moon = translate(movMat_moon, vec3(2, 0, 0));
    movMat_mars = translate(movMat_mars, vec3(14, 0, 0));

    // Viewport
    glViewport(0, 0, VIEW_SIZE_X, VIEW_SIZE_Y);
    GLfloat aspect = GLfloat(VIEW_SIZE_X) / VIEW_SIZE_Y;
    mat4 projection = perspective(45.0f, aspect, 1.0f, 500.0f);
    GLuint Projection = glGetUniformLocation(m_sphereProgramID, "projMatrix");
    glUniformMatrix4fv(Projection, 1, GL_FALSE, value_ptr(projection));

    // Shader 변수 ID 가져오기
    rquatID = glGetUniformLocation(m_sphereProgramID, "rquat");
    ModelMatrixID = glGetUniformLocation(m_sphereProgramID, "modelMatrix");
    DistanceMatrixID = glGetUniformLocation(m_sphereProgramID, "distanceMat_view");
    shading_switchID = glGetUniformLocation(m_sphereProgramID, "shading");
    texture_switchID = glGetUniformLocation(m_sphereProgramID, "texturing");
    glUniformMatrix4fv(DistanceMatrixID, 1, GL_FALSE, value_ptr(distanceMat_view));

    // Texture
    glEnable(GL_TEXTURE_2D);
    textureID = glGetUniformLocation(m_sphereProgramID, "texture");
    BG_texID = LoadTexture("2k_stars_milky_way.jpg", 0);
    sun_texID = LoadTexture("2k_sun.jpg", 1);
    earth_texID = LoadTexture("2k_earth_daymap.jpg", 2);
    moon_texID = LoadTexture("2k_moon.jpg", 3);
    mars_texID = LoadTexture("2k_mars.jpg", 4);

    glUniform1f(texture_switchID, 1);
}

void cleanUp() {
    sphere.cleanup();
}

float speed = 10;
//궤도 그리기 함수(반지름)
void DrawCircle(float radius) {
    glUniform1f(shading_switchID, 0);
    glUniform1f(texture_switchID, 0);
    glPointSize(1);
    glBegin(GL_POINTS);
    for (float theta = 0; theta <= 3.14 * 2.f; theta += 0.002) {
        float xpoint = radius * cos(theta); // 현재 매개변수에 해당하는 x 값 계산
        float ypoint = radius * sin(theta);  // 현재 매개변수에 해당하는 y 값 계산
        glVertex3f(xpoint, 0, ypoint); // 현재 매개변수로 얻은 좌표를 출력
    }
    glEnd();
    glUniform1f(shading_switchID, !lineMode);
    glUniform1f(texture_switchID, !lineMode);
}

void draw_axis()
{
    glUniform1f(shading_switchID, 0);
    glUniform1f(texture_switchID, 0);
    glPointSize(1);
    glBegin(GL_POINTS);
    for (float theta = -2; theta <= 2; theta += 0.002) {
        glVertex3f(0, theta, 0); // 현재 매개변수로 얻은 좌표를 출력
    }
    glEnd();
    glUniform1f(shading_switchID, !lineMode);
    glUniform1f(texture_switchID, !lineMode);
}

void draw_background()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, BG_texID);
    glUniform1i(textureID, 0);

    mat4 m = mat4(1);
    m = scale(m, vec3(80));
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, value_ptr(m));

    glUniform1f(shading_switchID, 0);
    sphere.draw();
    glUniform1f(shading_switchID, !lineMode);
}

void draw_sun()
{
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sun_texID);
    glUniform1i(textureID, 1);

    float angle_local = Deg2Rad * speed / 27;
    rotMat_sun_local = rotate(rotMat_sun_local, angle_local, vec3(0, 1, 0));

    mat4 m = mat4(1); 
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, value_ptr(m));
    DrawCircle(10);
    m = m * rotate(m, (float)(Deg2Rad * 5.65), vec3(0, 0, 1));
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, value_ptr(m));
    DrawCircle(14);

    m = rotMat_sun_local * scaleMat_sun;
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, value_ptr(m));
    draw_axis();

    glUniform1f(shading_switchID, 0);
    sphere.draw();
    glUniform1f(shading_switchID, !lineMode);
}

void draw_earth()
{ 
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, earth_texID);
    glUniform1i(textureID, 2);

    float angle_local = Deg2Rad * speed;
    float angle = Deg2Rad * speed / 365;
    rotMat_earth_local = rotate(rotMat_earth_local, angle_local, vec3(0, 1, 0));
    rotMat_earth = rotate(rotMat_earth, angle, vec3(0, 1, 0));

    // 지구 공전의 반대 방향 회전 행렬
    mat4 m = rotMat_earth;
    m[0][0] *= -1;
    m[2][2] *= -1;

    mat4 circle = mat4(1);
    circle = rotMat_earth * movMat_earth * rotate(circle,(float)(Deg2Rad * 5.1), vec3(0,0,1));
       
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, value_ptr(circle));
    DrawCircle(2);

    m = rotMat_earth * movMat_earth * m * rotMat_earth_local * scaleMat_earth;
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, value_ptr(m));
    draw_axis();

    sphere.draw();
}

void draw_moon()
{
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, moon_texID);
    glUniform1i(textureID, 3);

    float angle_local = Deg2Rad * speed / (float)27.3;
    float angle = angle_local;

    // 지구 공전의 반대 방향 회전 행렬
    mat4 m = rotMat_earth;
    m[0][0] *= -1;
    m[2][2] *= -1;

    // 달 공전의 반대 방향 회전 행렬
    mat4 m2 = rotMat_moon;
    m2[0][0] *= -1;
    m2[2][2] *= -1;
    rotMat_moon_local = rotate(rotMat_moon_local, angle_local, vec3(0, 1, 0));
    rotMat_moon = rotate(rotMat_moon, angle, vec3(0, 1, 0));
    m = rotMat_earth * movMat_earth * rotMat_moon * movMat_moon * m * m2 * rotMat_moon_local * scaleMat_moon;
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, value_ptr(m));
    draw_axis();

    sphere.draw();
}

void draw_mars()
{
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, mars_texID);
    glUniform1i(textureID, 4);

    float angle_local = Deg2Rad * speed;
    float angle = Deg2Rad * speed / 687;
    rotMat_mars_local = rotate(rotMat_mars_local, angle_local, vec3(0, 1, 0));
    rotMat_mars = rotate(rotMat_mars, angle, vec3(0, 1, 0));

    // 화성 공전의 반대 방향 회전 행렬
    mat4 m = rotMat_mars;
    m[0][0] *= -1;
    m[2][2] *= -1;

    m = rotMat_mars * movMat_mars * m * rotMat_mars_local * scaleMat_mars;
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, value_ptr(m));
    draw_axis();

    sphere.draw();
}

void display()
{

   /* glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_QUADS);
    glVertex2f(-0.5f, -0.5f);
    glVertex2f(-0.5f, 0.5f);
    glVertex2f(0.5f, 0.5f);
    glVertex2f(0.5f, -0.5f);
    glEnd();

    glFlush();
    return;*/

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // rquat는 공통으로 사용되므로 한 프레임에 한번만 전송
    glUniform4fv(rquatID, 1, value_ptr(rquat));
    draw_background();
    draw_sun();
    draw_earth();
    draw_moon();
    draw_mars();
   
    glutSwapBuffers();
}
int main(int argc, char** argv)
{

    //glutInit(&argc, argv);
    //glutCreateWindow("simple");
    //glutDisplayFunc(display);

    ///* Run the GLUT event loop */
    //glutMainLoop();

    //return EXIT_SUCCESS;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); //더블버퍼컬러모드.
    glutInitWindowSize(VIEW_SIZE_X, VIEW_SIZE_Y);
    glutCreateWindow("Solar System");
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glutDisplayFunc(display);
    glutTimerFunc(20, Tick, 1);
    
    glutMouseFunc(mouse);
    glutMouseWheelFunc(wheel);
    glutMotionFunc(mouseMotion);
    
    init();
    glutMainLoop();
    cleanUp();
    return EXIT_SUCCESS;
}