#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Key {
    W,
    S,
    A,
    D,
    SPACE,
    LEFT_CONTROL,
    TAB,
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
    float speed = 0.02f;
    float fov = 90.0f;

    Camera();
    Camera(const glm::vec3 &position, float yaw, float pitch);

    glm::mat4 getViewMatrix() const;

    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    void processKeyboard(int key, float deltaTime);

    void processScroll(float xoffset, float yoffset);
    
    void updateCameraVectors();
};

#endif
