#include "Camera.hpp"

Camera::Camera() : position(glm::vec3(0.0f, 0.0f, 0.0f)), yaw(0.0f), pitch(0.0f)
{
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    updateCameraVectors();
}

Camera::Camera(const glm::vec3 &position, float yaw, float pitch) : position(position), yaw(yaw), pitch(pitch)
{
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch -= yoffset;

    yaw += 360.0f * (yaw < -180.0f);
    yaw -= 360.0f * (yaw >  180.0f);

    const float LIMIT = 89.9f;

    if (constrainPitch)
    {
        if (pitch > LIMIT)
            pitch = LIMIT;
        if (pitch < -LIMIT)
            pitch = -LIMIT;
    }

    updateCameraVectors();
}

void Camera::processKeyboard(int key, float deltaTime) {
    glm::vec3 deltaPos = glm::vec3(0.0f);

    if (key == Key::W)
        deltaPos += front * speed;
    if (key == Key::S)
        deltaPos -= front * speed;
    if (key == Key::A)
        deltaPos -= right * speed;
    if (key == Key::D)
        deltaPos += right * speed;
    if (key == Key::SPACE)
        deltaPos += up * speed;
    if (key == Key::LEFT_CONTROL)
        deltaPos -= up * speed;

    position += deltaPos;

  updateCameraVectors();
}

void Camera::processScroll(float xoffset, float yoffset)
{
    fov += yoffset;

    if (fov > 160.0f)
        fov = 160.0f;

    if (fov < 20.0f)
        fov = 20.0f;
}

void Camera::updateCameraVectors()
{
    front = glm::normalize(glm::vec3(
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))));

    right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    up = glm::normalize(glm::cross(right, front));
}