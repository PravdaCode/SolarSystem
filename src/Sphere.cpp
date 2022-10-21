#include "Sphere.h"
#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <glm/gtx/transform.hpp>
#include "glm/gtc/matrix_transform.hpp"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

static const float periodOEarth = 10.0f;
static const float periodREarth = periodOEarth / 2;

static const float periodOMoon = periodREarth / 2;
static const float periodRMoon = periodREarth;

static glm::vec4 centerearth = glm::vec4(10.0f, 0.0f, 0.0f, 1.0f);

Sphere::Sphere(float r, float x, float y, float z) {
    this->radius = r;
    this->x = x;
    this->y = y;
    this->z = z;

}

void Sphere::init() {
    m_vao = 0;
    m_posVbo = 0;
    m_normalVbo = 0;
    m_ibo = 0;
    m_texVbo = 0;
    this->createVector(24);
}

void Sphere::initGPUgeometry() {
    // Create a single handle, vertex array object that contains attributes,
    // vertex buffer objects (e.g., vertex's position, normal, and color)
    #ifdef _MY_OPENGL_IS_33_
        glGenVertexArrays(1, &m_vao); // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.
    #else
        glCreateVertexArrays(1, &m_vao);
    #endif
        glBindVertexArray(m_vao);

        // Generate a GPU buffer to store the positions of the vertices
        size_t vertexBufferSize = sizeof(float) * this->m_vertexPositions.size(); // Gather the size of the buffer from the CPU-side vector

    #ifdef _MY_OPENGL_IS_33_
        glGenBuffers(1, &m_posVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
        glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, this->m_vertexPositions.data(), GL_DYNAMIC_READ);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);

        // Normals
        glGenBuffers(1, &this->m_normalVbo);
        glBindBuffer(GL_ARRAY_BUFFER, this->m_normalVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * this->m_vertexNormals.size(), this->m_vertexNormals.data(), GL_DYNAMIC_READ);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(1);

        // UV map
        glGenBuffers(1, &this->m_texCoordVbo);
        glBindBuffer(GL_ARRAY_BUFFER, this->m_texCoordVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * this->m_vertexTexCoords.size(), this->m_vertexTexCoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(2);


    #else
        glCreateBuffers(1, &g_posVbo);
        glBindBuffer(GL_ARRAY_BUFFER, g_posVbo);
        glNamedBufferStorage(g_posVbo, vertexBufferSize, this->m_vertexPositions.data(), GL_DYNAMIC_STORAGE_BIT); // Create a data storage on the GPU and fill it from a CPU array
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);
    #endif

        // Same for an index buffer object that stores the list of indices of the
        // triangles forming the mesh
        size_t indexBufferSize = sizeof(unsigned int) * this->m_triangleIndices.size();
    #ifdef _MY_OPENGL_IS_33_
        glGenBuffers(1, &m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, this->m_triangleIndices.data(), GL_DYNAMIC_READ);
    #else
        glCreateBuffers(1, &g_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
        glNamedBufferStorage(g_ibo, indexBufferSize, this->indices.data(), GL_DYNAMIC_STORAGE_BIT);
    #endif

        glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering
}

void Sphere::render(GLuint g_program, glm::mat4 modelMatrix, glm::mat4 viewMatrix, glm::mat4 transMatrix) {
    glActiveTexture(GL_TEXTURE0); // activate texture unit 0
    glBindTexture(GL_TEXTURE_2D, this->m_texVbo);
    glUniform1i(this->m_texVbo, 0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glUniformMatrix4fv(glGetUniformLocation(g_program, "modelMat"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix)); // compute the view matrix of the camera and pass it to the GPU program
    glUniformMatrix4fv(glGetUniformLocation(g_program, "transMat"), 1, GL_FALSE, glm::value_ptr(transMatrix)); // compute the projection matrix of the camera and pass it to the GPU program
    glBindVertexArray(this->m_vao);
    glDrawElements(GL_TRIANGLES, this->m_triangleIndices.size(), GL_UNSIGNED_INT, 0);

}



void Sphere::createVector(const size_t resolution) {
    float aux = M_PI / (resolution - 1);
    float theta, phi, x, y, z;
    int curvert = 0;
    float textureCoefficient = 1.0f / (resolution - 1);
    for (int i = 0; i < resolution; i++) {
        phi = M_PI - (2 * aux * i);
        for (int j = 0; j < resolution; j++) {
            theta = M_PI - (j * aux);

            x = radius * cos(phi) * sin(theta);
            z = radius * sin(phi) * sin(theta);
            y = radius * cos(theta);

            this->m_vertexPositions.push_back(x);
            this->m_vertexPositions.push_back(y);
            this->m_vertexPositions.push_back(z);

            this->m_vertexNormals.push_back(x);
            this->m_vertexNormals.push_back(y);
            this->m_vertexNormals.push_back(z);

            this->m_vertexTexCoords.push_back(i * textureCoefficient);
            this->m_vertexTexCoords.push_back(1-j * textureCoefficient);

            //std::cout << x << y << z << std::endl;

            if (i + 1 == resolution || j + 1 == resolution) {

            }
            //add indices (current vert + right + botton right)
            else {
                this->m_triangleIndices.push_back(curvert);
                this->m_triangleIndices.push_back(curvert + resolution);
                this->m_triangleIndices.push_back(curvert + resolution + 1);
                //add indices (current vert + down + botton right)
                this->m_triangleIndices.push_back(curvert);
                this->m_triangleIndices.push_back(curvert + resolution + 1);
                this->m_triangleIndices.push_back(curvert + 1);
            }

            curvert++;
        }
    }


}

std::vector<float> Sphere::getVector() {
    return this->m_vertexPositions;
}

std::string Sphere::getName() { return this->name; }

float Sphere::getPeriodO() { return this->periode_o; }
float Sphere::getPeriodR() { return this->periode_r; }

glm::vec4 Sphere::getPosition() { return glm::vec4(this->x, this->y, this->z, 1.0f); }

void Sphere::setName(std::string name) { this->name = name; }

void Sphere::setTex(GLuint tex) { this->m_texVbo = tex; }

void Sphere::setPeriode(float r, float o) {
    this->periode_r = r;
    this->periode_o = o;
}


