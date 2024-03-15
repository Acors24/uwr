#ifndef BUBBLE_HPP
#define BUBBLE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <epoxy/gl.h>

class Bubble
{
public:
    const static int AMOUNT = 500;
    static glm::vec4 positions[AMOUNT];
    static glm::vec4 colors[AMOUNT];

    enum Property {
        MAXRADIUS,
        RISINGSPEED,
        INFLATINGSPEED,
        SPECIAL
    };
    static glm::vec4 properties[AMOUNT];

    const static int RESOLUTION = 16;
    static glm::vec3 vertices[(RESOLUTION + 1) * 2 * RESOLUTION];

    static unsigned int VAO;
    static GLuint vertexBuffer, positionBuffer, colorBuffer, propertyBuffer;
    static unsigned int shaderProgram;

    static int difficulty;

    Bubble() = delete;

    static void init();

    static void initInstance(int id);

    static void setBuffers();
    static void setShaders();

    glm::mat4 getModelMatrix() const;

    static void drawAll(const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &cameraPos, const glm::vec3 &playerPos, bool fog);
    static void updateAll(float deltaTime, const glm::vec3 &cameraPos);
};

#endif