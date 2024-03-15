#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <glm/glm.hpp>

#include "AGL3Drawable.hpp"

class Player : AGLDrawable
{
    static const float vertices[642 * 3];

public:
    glm::vec3 position;
    float horizontalAngle;
    float verticalAngle;
    const float radius;

    Player(glm::vec3 position, float radius);

    void setBuffers();
    void setShaders();

    void draw(const glm::mat4 &view, const glm::mat4 &projection);
};

#endif