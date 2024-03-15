#ifndef VOLCANO_HPP
#define VOLCANO_HPP

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "AGL3Drawable.hpp"

class Volcano : AGLDrawable
{
public:
    static const int RESOLUTION = 20;
    static constexpr const float SCALE = 2.0f;
    static const int AMOUNT_OF_VERTICES = (RESOLUTION + 1) * 2 * 29 * 2;
    static glm::vec3 vertices[AMOUNT_OF_VERTICES];
    static glm::vec3 normals[AMOUNT_OF_VERTICES];

    glm::vec3 position;

    Volcano();

    void setBuffers();
    void setShaders();

    glm::mat4 getModelMatrix() const;

    void draw(const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &cameraPos, const glm::vec3 lightPositions[], const glm::vec3 &playerPos, bool fog);
};

#endif