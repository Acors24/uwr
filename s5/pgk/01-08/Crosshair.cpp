#include "Crosshair.hpp"
#include <cmath>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

Crosshair::Crosshair() {
  setShaders();
  setBuffers();
}

void Crosshair::setShaders() {
  compileShaders(R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require
         #extension GL_ARB_shading_language_420pack : require
         layout(location = 0) in vec3 vpos;

         void main(void) {
            gl_Position = vec4(vpos, 1.0f);
         }

      )END",
                 R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require

         out vec4 color;

         void main(void) {
            color = vec4(1.0f, 1.0f, 1.0f, 0.3f);
         } 

      )END");
}

void Crosshair::setBuffers() {
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

void Crosshair::draw() {
  bindProgram();
  bindBuffers();

  glLineWidth(1);
  glDrawArrays(GL_LINES, 0, sizeof(vertices) / sizeof(glm::vec3));
}

glm::vec3 Crosshair::vertices[] = {
    glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
};