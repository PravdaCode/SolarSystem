#ifndef SPHERE_H
#define SPHERE_H

#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>


class Sphere {

public:
    GLuint m_vao;
    GLuint m_posVbo;
    GLuint m_normalVbo;
    GLuint m_ibo;
    GLuint m_texVbo;
    std::vector<unsigned int> m_triangleIndices;
    std::vector<float> m_vertexPositions;
    std::vector<float> m_vertexNormals;
    std::vector<float> m_vertexTex;
    std::string name;
    float periode_r, periode_o;
    float radius;
    float x, y, z;

    Sphere(float r, float x, float y, float z);

    void init();

    void initGPUgeometry();

    void render(GLuint g_program, glm::mat4 modelMatrix, glm::mat4 viewMatrix, glm::mat4 projMatrix);

    void createVector(const size_t resolution);

    std::vector<float> getVector();

    void setName(std::string name);

    void setPeriode(float r, float o);

    static std::shared_ptr<Sphere> genSphere(const size_t resolution, float r);

    void update();

private:


};

#endif