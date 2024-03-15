#ifndef MODEL_HPP
#define MODEL_HPP

#include "ModelData.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <epoxy/gl.h>
#include <string>
#include <vector>

class Model
{
public:
    // glm::vec3 *vertices;
    // glm::vec3 *normals;
    // unsigned int *indices;
    ModelData data;

    // int nVertices;
    // int nNormals;
    // int nIndices;

    GLuint vao;
    GLuint vbo;
    GLuint nbo;
    GLuint ebo;
    GLuint ubo;
    GLuint mbo;
    static unsigned int shaderProgram;

    Model(const std::string &path);

    void setBuffers();
    static void setShaders();

    glm::mat4 getModelMatrix() const;

    void draw(const glm::mat4 &view, const glm::mat4 &projection);
};

#endif