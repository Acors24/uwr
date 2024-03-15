#include "Camera.hpp"
#include "EarthGrid.hpp"
#include <cmath>
#include <glm/trigonometric.hpp>

Camera::Camera() : Camera(glm::vec3(0.0f)) {}

Camera::Camera(const glm::vec3 &position) : position(position) {
  yaw = 0.0f;
  pitch = 0.0f;
  speed = 0.2f;
  longitude = 0.0f;
  latitude = 0.0f;
  altitude = 0.0f;
  updateCameraVectors(false);
}

glm::mat4 Camera::getViewMatrix() const {
  return glm::lookAt(position, position + front, up);
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool flat, bool slow,
                                  bool constrainPitch) {
  xoffset *= sensitivity * (slow ? 0.05f : 1.0f);
  yoffset *= sensitivity * (slow ? 0.05f : 1.0f);

  if (!flat) {
    yaw += xoffset;
    pitch -= yoffset;

    yaw += 360.0f * (yaw < -180.0f);
    yaw -= 360.0f * (yaw > 180.0f);

    const float LIMIT = 89.9f;

    if (constrainPitch) {
      if (pitch > LIMIT)
        pitch = LIMIT;
      if (pitch < -LIMIT)
        pitch = -LIMIT;
    }
  } else {
    longitude += xoffset * 0.1f;
    latitude -= yoffset * 0.1f;

    if (longitude < 180.0f)
      longitude += 360.0f;
    if (longitude > 180.0f)
      longitude -= 360.0f;

    const float latitudeLimit = glm::degrees(std::tan(glm::radians(85.0f)));
    if (latitude < -latitudeLimit)
      latitude = -latitudeLimit;
    if (latitude > latitudeLimit)
      latitude = latitudeLimit;

    position.x = longitude;
    position.y = latitude;
  }

  updateCameraVectors(flat);
}

void Camera::processScroll(float xoffset, float yoffset, bool flat) {
  fov += yoffset;

  if (fov > 160.0f)
    fov = 160.0f;

  if (fov < 10.0f)
    fov = 10.0f;
}

void Camera::processKeyboard(int key, float deltaTime, bool flat, bool fast) {
  if (!flat) {
    glm::vec3 deltaPos = glm::vec3(0.0f);

    glm::vec3 otherFront = glm::cross(up, right);

    if (key == Key::W)
      deltaPos += otherFront * speed;
    if (key == Key::S)
      deltaPos -= otherFront * speed;
    if (key == Key::A)
      deltaPos -= right * speed;
    if (key == Key::D)
      deltaPos += right * speed;
    if (key == Key::SPACE)
      deltaPos += up * speed;
    if (key == Key::LEFT_CONTROL)
      deltaPos -= up * speed;

    position += deltaPos * (fast ? 10.0f : 1.0f);
  }

  updateCameraVectors(flat);
}

void Camera::updateCameraVectors(bool flat) {
  front = glm::normalize(
      glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))));
  
  if (flat) {
    up = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
  } else {
    up = glm::normalize(position);
  }
  right = glm::normalize(glm::cross(front, up));
}

void Camera::setMode(bool flat) {
  if (flat) {
    EarthGrid::cartesianToSpherical(position.x, position.y, position.z,
                                    longitude, latitude, altitude);
    longitude = glm::degrees(longitude);
    latitude = glm::degrees(latitude);
    latitude = glm::degrees(std::tan(glm::radians(latitude)));
    altitude -= EarthGrid::EARTH_RADIUS;
    position.x = longitude;
    position.y = latitude;
    position.z = 0.0f;
    yaw = -90.0f;
    pitch = 0.0f;
  } else {
    latitude = glm::degrees(std::atan(glm::radians(latitude)));
    EarthGrid::sphericalToCartesian(
        glm::radians(longitude), glm::radians(latitude),
        EarthGrid::EARTH_RADIUS + altitude, position.x, position.y, position.z);
  }
  updateCameraVectors(flat);
}