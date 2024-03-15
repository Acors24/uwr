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
    float speed;
    float sensitivity = 0.05f;
    float fov = 90.0f;

    float longitude, latitude, altitude;

    Camera();
    Camera(const glm::vec3 &position);

    glm::mat4 getViewMatrix() const;

    void processMouseMovement(float xoffset, float yoffset, bool flat, bool slow, bool constrainPitch);

    void processScroll(float xoffset, float yoffset, bool flat);

    void processKeyboard(int key, float deltaTime, bool flat, bool fast);
    
    void updateCameraVectors(bool flat);

    void setMode(bool flat);
};

#endif
