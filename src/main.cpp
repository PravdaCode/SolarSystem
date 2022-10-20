// ----------------------------------------------------------------------------
// main.cpp
//
//  Created on: 24 Jul 2020
//      Author: Kiwon Um
//        Mail: kiwon.um@telecom-paris.fr
//
// Description: IGR201 Practical; OpenGL and Shaders (DO NOT distribute!)
//
// Copyright 2020-2022 Kiwon Um
//
// The copyright to the computer program(s) herein is the property of Kiwon Um,
// Telecom Paris, France. The program(s) may be used and/or copied only with
// the written permission of Kiwon Um or in accordance with the terms and
// conditions stipulated in the agreement/contract under which the program(s)
// have been supplied.
// ----------------------------------------------------------------------------

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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Sphere.h"

#include <filesystem>

// constants
const static float kSizeSun = 1;
const static float kSizeEarth = 0.5;
const static float kSizeMoon = 0.25;
const static float kRadOrbitEarth = 10;
const static float kRadOrbitMoon = 2;

std::vector<Sphere* > solarS;

// Window parameters
GLFWwindow *g_window = nullptr;

// GPU objects
GLuint g_program = 0; // A GPU program contains at least a vertex shader and a fragment shader

// OpenGL identifiers
GLuint g_vao = 0;
GLuint g_posVbo = 0;
GLuint g_ibo = 0;

GLuint c_vao = 0;
GLuint c_posVbo = 0;

// All vertex positions packed in one array [x0, y0, z0, x1, y1, z1, ...]
std::vector<float> g_vertexPositions;
std::vector<float> g_vertexColors;
// All triangle indices packed in one array [v00, v01, v02, v10, v11, v12, ...] with vij the index of j-th vertex of the i-th triangle
std::vector<unsigned int> g_triangleIndices;

// Basic camera model
class Camera {
public:
  inline float getFov() const { return m_fov; }
  inline void setFoV(const float f) { m_fov = f; }
  inline float getAspectRatio() const { return m_aspectRatio; }
  inline void setAspectRatio(const float a) { m_aspectRatio = a; }
  inline float getNear() const { return m_near; }
  inline void setNear(const float n) { m_near = n; }
  inline float getFar() const { return m_far; }
  inline void setFar(const float n) { m_far = n; }
  inline void setPosition(const glm::vec3 &p) { m_pos = p; }
  inline glm::vec3 getPosition() { return m_pos; }

  inline glm::mat4 computeViewMatrix() const {
    return glm::lookAt(m_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  }

  // Returns the projection matrix stemming from the camera intrinsic parameter.
  inline glm::mat4 computeProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
  }

private:
  glm::vec3 m_pos = glm::vec3(0, 0, 0);
  float m_fov = 45.f;        // Field of view, in degrees
  float m_aspectRatio = 1.f; // Ratio between the width and the height of the image
  float m_near = 0.1f; // Distance before which geometry is excluded from the rasterization process
  float m_far = 10.f; // Distance after which the geometry is excluded from the rasterization process
};
Camera g_camera;


GLuint loadTextureFromFileToGPU(const std::string &filename) {
  GLuint texID;
  int width, height, numComponents;
  // Loading the image in CPU memory using stb_image
  unsigned char *data = stbi_load(
    filename.c_str(),
    &width, &height,
    &numComponents, // 1 for a 8 bit grey-scale image, 3 for 24bits RGB image, 4 for 32bits RGBA image
      0);

  glGenTextures(1, &texID); // generate an OpenGL texture container
  glBindTexture(GL_TEXTURE_2D, texID); // activate the texture
  // Setup the texture filtering option and repeat mode; check www.opengl.org for details.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // Fill the GPU texture with the data stored in the CPU image
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  // TODO: create a texture and upload the image data in GPU memory using
  // glGenTextures, glBindTexture, glTexParameteri, and glTexImage2D

  // Free useless CPU memory
  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture

  return texID;
}

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow* window, int width, int height) {
  g_camera.setAspectRatio(static_cast<float>(width)/static_cast<float>(height));
  glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

// Executed each time a key is entered.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if(action == GLFW_PRESS && key == GLFW_KEY_W) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else if(action == GLFW_PRESS && key == GLFW_KEY_F) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  } else if(action == GLFW_PRESS && (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)) {
    glfwSetWindowShouldClose(window, true); // Closes the application if the escape key is pressed
  }
}

void errorCallback(int error, const char *desc) {
  std::cout <<  "Error " << error << ": " << desc << std::endl;
}

void initGLFW() {
  glfwSetErrorCallback(errorCallback);

  // Initialize GLFW, the library responsible for window management
  if(!glfwInit()) {
    std::cerr << "ERROR: Failed to init GLFW" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Before creating the window, set some option flags
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  // Create the window
  g_window = glfwCreateWindow(
    1024, 768,
    "Interactive 3D Applications (OpenGL) - Simple Solar System",
    nullptr, nullptr);
  if(!g_window) {
    std::cerr << "ERROR: Failed to open window" << std::endl;
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
  glfwMakeContextCurrent(g_window);
  glfwSetWindowSizeCallback(g_window, windowSizeCallback);
  glfwSetKeyCallback(g_window, keyCallback);
}

void initOpenGL() {
  // Load extensions for modern OpenGL
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  glCullFace(GL_BACK); // Specifies the faces to cull (here the ones pointing away from the camera)
  glEnable(GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
  glDepthFunc(GL_LESS);   // Specify the depth test for the z-buffer
  glEnable(GL_DEPTH_TEST);      // Enable the z-buffer test in the rasterization
  glClearColor(0.7f, 0.7f, 0.7f, 1.0f); // specify the background color, used any time the framebuffer is cleared
}

// Loads the content of an ASCII file in a standard C++ string
std::string file2String(const std::string &filename) {
  std::ifstream t(filename.c_str());
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

// Loads and compile a shader, before attaching it to a program
void loadShader(GLuint program, GLenum type, const std::string &shaderFilename) {
  GLuint shader = glCreateShader(type); // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
  std::string shaderSourceString = file2String(shaderFilename); // Loads the shader source from a file to a C++ string
  const GLchar *shaderSource = (const GLchar *)shaderSourceString.c_str(); // Interface the C++ string through a C pointer
  glShaderSource(shader, 1, &shaderSource, NULL); // load the vertex shader code
  glCompileShader(shader);
  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cout << "ERROR in compiling " << shaderFilename << "\n\t" << infoLog << std::endl;
  }
  glAttachShader(program, shader);
  glDeleteShader(shader);
}

void initGPUprogram() {
  g_program = glCreateProgram(); // Create a GPU program, i.e., two central shaders of the graphics pipeline
  loadShader(g_program, GL_VERTEX_SHADER, "vertexShader.glsl");
  loadShader(g_program, GL_FRAGMENT_SHADER, "fragmentShader.glsl");
  glLinkProgram(g_program); // The main GPU program is ready to be handle streams of polygons

  glUseProgram(g_program);
  // TODO: set shader variables, textures, etc.
}

// Define your mesh(es) in the CPU memory
void initCPUgeometry() {
  // TODO: add vertices and indices for your mesh(es)
  g_vertexPositions = { 0.f, 0.f, 0.f,
                        1.f, 0.f, 0.f,
                        0.f, 1.f, 0.f,
                        1.f, 1.f, 1.f};

  g_triangleIndices = {0,1,2,0,1,3};


}

void initGPUgeometry() {
  // Create a single handle, vertex array object that contains attributes,
  // vertex buffer objects (e.g., vertex's position, normal, and color)
#ifdef _MY_OPENGL_IS_33_
  glGenVertexArrays(1, &g_vao);
#else
  glCreateVertexArrays(1, &g_vao);
#endif
  glBindVertexArray(g_vao);

  // Generate a GPU buffer to store the positions of the vertices
  size_t vertexBufferSize = sizeof(float)* g_vertexPositions.size(); // Gather the size of the buffer from the CPU-side vector
  //size_t colorBufferSize = sizeof(float) * g_vertexColors.size();
#ifdef _MY_OPENGL_IS_33_
  glGenBuffers(1, &g_posVbo);
  glBindBuffer(GL_ARRAY_BUFFER, g_posVbo);
  glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, g_vertexPositions.data(), GL_DYNAMIC_READ);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
  glEnableVertexAttribArray(0);
#else
  glCreateBuffers(1, &sphere->m_posVbo);
  glBindBuffer(GL_ARRAY_BUFFER, sphere->m_posVbo);
  glNamedBufferStorage(g_posVbo, vertexBufferSize, sphere->getVector().data(), GL_DYNAMIC_STORAGE_BIT); // Create a data storage on the GPU and fill it from a CPU array
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
  glEnableVertexAttribArray(0);
#endif


  // Same for an index buffer object that stores the list of indices of the
  // triangles forming the mesh
  size_t indexBufferSize = sizeof(unsigned int)*g_triangleIndices.size();
#ifdef _MY_OPENGL_IS_33_
  glGenBuffers(1, &g_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, g_triangleIndices.data(), GL_DYNAMIC_READ);
#else
  glCreateBuffers(1, &sphere->m_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere->m_ibo);
  glNamedBufferStorage(sphere->m_ibo, indexBufferSize, sphere->m_triangleIndices.data(), GL_DYNAMIC_STORAGE_BIT);
#endif


  glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering
}

void initCamera() {
  int width, height;
  glfwGetWindowSize(g_window, &width, &height);
  g_camera.setAspectRatio(static_cast<float>(width)/static_cast<float>(height));

  g_camera.setPosition(glm::vec3(5.0, 2, 20));
  g_camera.setNear(0.1);
  g_camera.setFar(80.1);
}

void init() {
  initGLFW();
  initOpenGL();
  for (Sphere* s : solarS) {
      s->init();
  }

  for (Sphere* s : solarS) {
      s->initGPUgeometry();
  }
  initGPUprogram();
  initCamera();
}

void clear() {
  glDeleteProgram(g_program);

  glfwDestroyWindow(g_window);
  glfwTerminate();
}

// The main rendering call
void render(std::vector<Sphere *> spheres) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

  const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
  const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();
  glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix)); // compute the view matrix of the camera and pass it to the GPU program
  glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(projMatrix)); // compute the projection matrix of the camera and pass it to the GPU program

  for (Sphere* s : solarS) {
      s->update();
      s->render();
  }
 
  const float currentTime = glfwGetTime();
  const float periodOEarth = 10.0f;
  const float periodREarth = periodOEarth / 2;

  const float periodOMoon = periodREarth / 2;
  const float periodRMoon = periodREarth;


  glm::mat4 modelMatrix(1.f);
  glm::mat4 transformationMatrix = projMatrix * viewMatrix * modelMatrix;
  glUniform1i(glGetUniformLocation(g_program, "sunFlag"), 1);
  solarS[0]->render(g_program, modelMatrix, viewMatrix, transformationMatrix);


 float speed;

   //earth
GLuint m_vao = solarS[1]->m_vao;
glm::vec4 center = glm::vec4(10.0f,0.0f,0.0f, 1.0f);
          //orbit

speed = 360.0f / periodOEarth;
float orbitangleearth = speed * currentTime;

glm::mat4 orbitMat = glm::rotate(glm::radians(orbitangleearth), glm::vec3(0.0f, 1.0f, 0.0f));
center = orbitMat * center;
glm::vec3 centerearth = glm::vec3(center);

modelMatrix = glm::translate(centerearth);


          //rotation
speed = 360 / periodRMoon;
float rotationanglemoon = speed * currentTime;

glm::mat4 rotateMatrix = glm::rotate(glm::radians(rotationanglemoon), glm::vec3(0.0f, 1.0f, 0.0f));
rotateMatrix = rotateMatrix * glm::rotate(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
modelMatrix = modelMatrix * rotateMatrix;

transformationMatrix = projMatrix * viewMatrix * modelMatrix;
solarS[1]->render(g_program, modelMatrix, viewMatrix, transformationMatrix);


          //moon
          // orbit
          speed = 360 / periodOMoon;
          float orbitanglemoon = speed * currentTime;
          center = glm::vec4(2.0f, 0.0f, 0.0f, 1.0f);
          glm::mat4 orbitMatrix = glm::rotate(glm::radians(orbitanglemoon), glm::vec3(0.0f, 1.0f, 0.0f));
          center = orbitMatrix * center;
          glm::vec3 vec3CenterLua = glm::vec3(center);
          vec3CenterLua = vec3CenterLua + centerearth;

          modelMatrix = glm::translate(vec3CenterLua);

          speed = 360 / periodREarth;
          float rotationangleearth = speed * currentTime;

          rotateMatrix = glm::rotate(glm::radians(rotationangleearth), glm::vec3(0.0f, 1.0f, 0.0f));
          rotateMatrix = rotateMatrix * glm::rotate(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
          
          modelMatrix = modelMatrix * rotateMatrix;

          transformationMatrix = projMatrix * viewMatrix * modelMatrix;
          solarS[2]->render(g_program, modelMatrix, viewMatrix, transformationMatrix);
          glUniform1i(glGetUniformLocation(g_program, "sunFlag"), 0);


  const glm::vec3 camPosition = g_camera.getPosition();
  glUniform3f(glGetUniformLocation(g_program, "camPos"), camPosition[0], camPosition[1], camPosition[2]);

  
}

// Update any accessible variable based on the current time
void update(const float currentTimeInSec) {
  // std::cout << currentTimeInSec << std::endl;

}

#include <direct.h>
#define GetCurrentDir _getcwd

int main(int argc, char ** argv) {
    char buff[FILENAME_MAX]; //create string buffer to hold path
    GetCurrentDir(buff, FILENAME_MAX);
    std::string current_working_dir(buff);

  Sphere* sun = new Sphere(kSizeSun, 0, 0, 0);
  Sphere* earth = new Sphere(kSizeEarth*2, kRadOrbitEarth, 0, 0);
  Sphere* moon = new Sphere(kSizeMoon, (kRadOrbitEarth+kRadOrbitMoon)/2, 0, 0);

  //GLint g_earthTextID = loadTextureFromFileToGPU("media/earth.jpg");


  sun->setName("sun");
  earth->setName("earth");
  moon->setName("moon");

  earth->setPeriode(10,10);
  moon->setPeriode(2,4);



  solarS.push_back(sun);
  solarS.push_back(earth);
  solarS.push_back(moon);

  init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
  while(!glfwWindowShouldClose(g_window)) {
    update(static_cast<float>(glfwGetTime()));
    render(solarS);
    glfwSwapBuffers(g_window);
    glfwPollEvents();
  }
  clear();
  return EXIT_SUCCESS;
}
