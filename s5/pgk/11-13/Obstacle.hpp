#ifndef OBSTACLE_HPP
#define OBSTACLE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "AGL3Drawable.hpp"

class Obstacle : public AGLDrawable
{
public:
    glm::vec3 position;
    glm::vec4 rotation;
    glm::vec4 color;
    static float scale;
    float colliding = 0.0f;
    static const float vertices[6 * 4];

    Obstacle(float x, float y, float z);

    void setBuffers();
    void setShaders();

    glm::mat4 getModelMatrix() const;

    void draw(const glm::mat4 &view, const glm::mat4 &projection, float collapse_time, bool hollow = false, bool fog = true);

    void setPosition(float x, float y, float z);
};

class Goal : public Obstacle
{
public:
    static const float vertices[6 * 4 + 6 * 2];

    Goal(float x, float y, float z);

    void setBuffers();
    void setShaders();

    void draw(const glm::mat4 &view, const glm::mat4 &projection, float collapse_time, bool hollow = false, bool fog = true);
};


#endif