#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ClosestPointTriangle.hpp"

#include <vector>

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
    float speed = 0.1f;
    float sensitivity = 0.02f;
    float fov = 120.0f;

    Camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        float yaw = 0.0f,
        float pitch = 0.0f,
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) : position(position), yaw(yaw), pitch(pitch), up(up), front(glm::vec3(0.0f, 0.0f, -1.0f)), right(glm::vec3(1.0f, 0.0f, 0.0f))
    {
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix() const
    {
        return glm::lookAt(position, position + front, up);
    }

    void processKeyboard(Camera_Movement direction, float deltaTime, std::vector<Obstacle> &obstacles, Box &box, Goal &goal, const Player &player, bool &finished)
    {
        float velocity = speed * deltaTime;
        glm::vec3 posDelta = {0, 0, 0};

        if (direction == FORWARD)
            posDelta += front * velocity;
        if (direction == BACKWARD)
            posDelta -= front * velocity;
        if (direction == LEFT)
            posDelta -= right * velocity;
        if (direction == RIGHT)
            posDelta += right * velocity;
        if (direction == UP)
            posDelta += up * velocity;
        if (direction == DOWN)
            posDelta -= up * velocity;

        float distance;
        for (Obstacle &obs : obstacles)
        {
            distance = distanceToObstacle(position + posDelta, obs);
            if (distance < player.radius)
            {
                obs.colliding = 1.0f;
                return;
            }
            obs.colliding = 0.0f;
        }

        distance = distanceToBox(position + posDelta, box);
        if (distance < player.radius)
        {
            return;
        }

        distance = distanceToGoal(position + posDelta, goal);
        if (distance < player.radius)
        {
            finished = true;
            goal.colliding = 1.0f;
            return;
        }

        position += posDelta;

        float arrowSensitivity = 1000.0f;
        if (direction == LOOK_UP)
            processMouseMovement(0, -sensitivity * arrowSensitivity);
        if (direction == LOOK_DOWN)
            processMouseMovement(0, sensitivity * arrowSensitivity);
        if (direction == LOOK_LEFT)
            processMouseMovement(-sensitivity * arrowSensitivity, 0);
        if (direction == LOOK_RIGHT)
            processMouseMovement(sensitivity * arrowSensitivity, 0);
    }

    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch -= yoffset;

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

    void processScroll(float xoffset, float yoffset)
    {
        fov += yoffset;

        if (fov > 160.0f)
            fov = 160.0f;

        if (fov < 20.0f)
            fov = 20.0f;
    }
    
    void updateCameraVectors()
    {
        front = glm::normalize(glm::vec3(
            cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
            sin(glm::radians(pitch)),
            sin(glm::radians(yaw)) * cos(glm::radians(pitch))));

        right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        up = glm::normalize(glm::cross(right, front));
    }
};

#endif
