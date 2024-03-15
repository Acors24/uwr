#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Player;

enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
    LOOK_UP,
    LOOK_DOWN,
    LOOK_LEFT,
    LOOK_RIGHT
};

class Camera
{
public:
    glm::vec3 position;
    glm::vec3 up;
    glm::vec3 front;
    glm::vec3 right;
    float yaw;
    float pitch;
    float sensitivity = 0.05f;
    float fov = 90.0f;
    Player *player;

    Camera(Player *player);
    Camera(const glm::vec3 &position, float yaw, float pitch);

    glm::mat4 getViewMatrix() const;

    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    void processScroll(float xoffset, float yoffset);
    
    void updateCameraVectors();
};

#endif
