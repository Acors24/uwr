#ifndef EARTHGRID_HPP
#define EARTHGRID_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "AGL3Drawable.hpp"

class EarthGrid : public AGLDrawable {
public:
  constexpr static float EARTH_RADIUS = 6378.0f;
  const static int RESOLUTION = 180;
  static glm::vec3 vertices[(RESOLUTION + 1) * 2 * 2 * RESOLUTION];

  EarthGrid();

  void setBuffers();
  void setShaders();

  glm::mat4 getModelMatrix() const;

  void draw(const glm::mat4 &view, const glm::mat4 &projection);

  static void sphericalToCartesian(float phi, float theta, float r,
                                   float &out_x, float &out_y, float &out_z);

  static void cartesianToSpherical(float x, float y, float z,
                                   float &out_phi, float &out_theta, float &out_r);
};

#endif