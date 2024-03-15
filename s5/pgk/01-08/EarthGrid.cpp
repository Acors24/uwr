#include "EarthGrid.hpp"
#include <cmath>
#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

static int initMesh() {
  int i = 0;
  const int RES = EarthGrid::RESOLUTION;
  float x, y, z, hTheta, vTheta, vTheta2, radius;
  for (int j = 0; j < RES; j++) {
    vTheta = j * M_PI / RES;
    vTheta2 = (j + 1) * M_PI / RES;

    for (int k = 0; k <= 2 * RES; k++) {
      hTheta = k * 2 * M_PI / (2 * RES);
      radius = std::sin(vTheta);

      x = std::cos(hTheta) * radius;
      y = std::cos(vTheta);
      z = std::sin(hTheta) * radius;

      EarthGrid::vertices[i++] = glm::vec3(x, y, z);

      radius = std::sin(vTheta2);

      x = std::cos(hTheta) * radius;
      y = std::cos(vTheta2);
      z = std::sin(hTheta) * radius;

      EarthGrid::vertices[i++] = glm::vec3(x, y, z);
    }
  }

  return 0;
}

EarthGrid::EarthGrid() {
  static int _init = initMesh();

  setShaders();
  setBuffers();
}

void EarthGrid::setShaders() {
  compileShaders(R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require
         #extension GL_ARB_shading_language_420pack : require
         layout(location = 0) in vec3 vpos;
         layout(location = 0) uniform mat4 model;
         layout(location = 1) uniform mat4 view;
         layout(location = 2) uniform mat4 projection;

         void main(void) {
            gl_Position = projection * view * model * vec4(vpos, 1.0f);
         }

      )END",
                 R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require

         out vec4 color;

         void main(void) {
            color = vec4(1.0f, 1.0f, 1.0f, 0.1f);
         } 

      )END");
}

void EarthGrid::setBuffers() {
  bindBuffers();

  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, // attribute 0, must match the layout in the shader.
                        3, // size
                        GL_FLOAT, // type
                        GL_FALSE, // normalized?
                        0,        // 24,             // stride
                        (void *)0 // array buffer offset
  );
}

glm::mat4 EarthGrid::getModelMatrix() const {
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(1.0f, 1.0f, 1.0f));
  model = glm::scale(model, glm::vec3(EARTH_RADIUS));

  return model;
}

void EarthGrid::draw(const glm::mat4 &view, const glm::mat4 &projection) {
  glm::mat4 model = getModelMatrix();

  bindProgram();
  bindBuffers();
  glUniformMatrix4fv(0, 1, GL_FALSE, &model[0][0]);
  glUniformMatrix4fv(1, 1, GL_FALSE, &view[0][0]);
  glUniformMatrix4fv(2, 1, GL_FALSE, &projection[0][0]);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glLineWidth(2);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(vertices) / sizeof(glm::vec3));
}

void EarthGrid::sphericalToCartesian(float phi, float theta, float r,
                                     float &out_x, float &out_y, float &out_z) {
  out_x = r * -cos(theta) * cos(phi);
  out_y = r * sin(theta);
  out_z = r * cos(theta) * sin(phi);
  
}

void EarthGrid::cartesianToSpherical(float x, float y, float z, float &out_phi,
                                 float &out_theta, float &out_r) {
  out_r = std::sqrt(x * x + y * y + z * z);
  out_theta = std::asin(y / out_r);
  out_phi = std::acos(x / out_r / -std::cos(out_theta)) * glm::sign(z);
}

glm::vec3 EarthGrid::vertices[];