#ifndef CROSSHAIR_HPP
#define CROSSHAIR_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "AGL3Drawable.hpp"

class Crosshair : public AGLDrawable {
public:
  static glm::vec3 vertices[4];

  Crosshair();

  void setBuffers();
  void setShaders();

  void draw();
};

#endif