#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <glm/glm.hpp>

#include "AGL3Drawable.hpp"
#include "Volcano.hpp"

class Camera;
class Bubble;

enum Key {
    W,
    S,
    A,
    D,
    SPACE,
    LEFT_CONTROL,
    TAB,
};

class Player : AGLDrawable
{
public:
    glm::vec3 position;
    glm::vec3 desiredPosition;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    float yaw;
    float desiredYaw;
    float yawSpeed;
    float pitch;
    float speed;
    Camera *camera;
    glm::vec3 cameraPosition;
    bool tpCamera;
    bool dead;
    unsigned points;
    unsigned level;

    const static int RESOLUTION = 16;
    static glm::vec3 vertices[(RESOLUTION + 1) * 2 * RESOLUTION];

    Player(glm::vec3 position, float yaw);

    void setBuffers();
    void setShaders();

    glm::mat4 getModelMatrix(int sphere_index, bool bigger) const;

    void draw(const glm::mat4 &view, const glm::mat4 &projection, bool highlight);

    void processKeyboard(int key, float deltaTime);

    void update(float deltaTime, const glm::vec4 bubblePositions[], const Volcano &volcano);

    void updateCameraPosition();

    bool bubbleCollision(const glm::vec3 &newPos, const glm::vec4 &bubblePos) const;

    bool volcanoCollision(const glm::vec3 &newPos, const Volcano &volcano) const;
};

#endif