#ifndef SUN_HPP
#define SUN_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "AGL3Drawable.hpp"

class Sun : public AGLDrawable
{
public:
    glm::vec3 position;
    float radius;
    const static int RESOLUTION = 20;
    static glm::vec3 vertices[(RESOLUTION + 1) * 2 * RESOLUTION];
    glm::vec3 color;

    Sun();

    void setBuffers();
    void setShaders();

    glm::mat4 getModelMatrix() const;

    void draw(const glm::mat4 &view, const glm::mat4 &projection);
};

#endif