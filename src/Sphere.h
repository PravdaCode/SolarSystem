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

    static glm::vec4 centerearth;

public:

    Sphere(float r, float x, float y, float z);

    void init();

    void initGPUgeometry();

    void render(GLuint g_program, glm::mat4 modelMatrix, glm::mat4 viewMatrix, glm::mat4 projMatrix);

    void createVector(const size_t resolution);

    std::vector<float> getVector();

    std::string getName();

    float Sphere::getPeriodO();
    float Sphere::getPeriodR();

    glm::vec4 getPosition();

    void setName(std::string name);

    void setPeriode(float r, float o);

    void setTex(GLuint tex);

    static std::shared_ptr<Sphere> genSphere(const size_t resolution, float r);

private:

    //vao and coords related to texture
    std::vector<float> m_vertexTexCoords;
    GLuint m_texVbo;
    GLuint m_texCoordVbo;

    //vao related to the Sphere
    std::string name;
    GLuint m_vao;
    GLuint m_posVbo;
    GLuint m_normalVbo;
    GLuint m_ibo;

    //vectors to store the informations on each vertice and the conection between them
    std::vector<unsigned int> m_triangleIndices;
    std::vector<float> m_vertexPositions;
    std::vector<float> m_vertexNormals;

    //physical properties of the spheres
    float periode_r, periode_o;
    float radius;
    float x, y, z;

};

#endif