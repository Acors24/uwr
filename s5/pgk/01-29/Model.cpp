#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "Model.hpp"
#include "Util.hpp"

Model::Model(const std::string &path) {
  data = util::loadObj(path);
  
  setBuffers();
  setShaders();
}

void Model::setShaders() {
  const char *vertexShaderSource = R"END(
         #version 330
         #extension GL_ARB_explicit_uniform_location : require
         #extension GL_ARB_shading_language_420pack : require

         layout(location = 0) in vec3 vPos;
         layout(location = 1) in vec3 vNormal;
         layout(location = 2) in int vMId;
         
         layout(location = 0) uniform mat4 view;
         layout(location = 1) uniform mat4 projection;

         out vec3 N;
         out vec3 L;
         out vec3 V;
         flat out int mId;

         void main(void) {
            gl_Position = projection * view * vec4(vPos, 1.0f);
            N = vNormal;
            L = vec3(1.0f, 1.0f, 1.0f);
            vec4 cP = inverse(projection * view) * vec4(0,0,-1,0);
            V = cP.xyz / cP.w - vPos;

            mId = vMId;
         }
      )END";
  const char *fs = R"END(
         #version 330
         #extension GL_ARB_explicit_uniform_location : require

         struct Material {
             vec3 Ka;
             vec3 Kd;
             vec3 Ks;
             float Ns;
             float Ni;
             float d;
         };

         uniform Material materials[32];

         in vec3 N;
         in vec3 L;
         in vec3 V;
         flat in int mId;

         out vec4 color;

         void main(void) {
          vec3 N = normalize(N);
          vec3 L = normalize(L);
          vec3 V = normalize(V);

          vec3 Ka = materials[mId].Ka;
          vec3 Kd = materials[mId].Kd;
          vec3 Ks = materials[mId].Ks;
          float Ns = materials[mId].Ns;
          float d = materials[mId].d;

          vec3 is = vec3(1.0f, 1.0f, 1.0f);
          vec3 id = vec3(1.0f, 1.0f, 1.0f);
          vec3 ia = vec3(0.2f);

          vec3 R = 2.0f * dot(L, N) * N - L;
          vec3 I = Ka * ia + Kd * max(dot(L, N), 0) * id + Ks * pow(max(dot(R, V), 0), Ns) * is;

          color = vec4(I, d);
         }
      )END";

  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  glShaderSource(fragmentShader, 1, &fs, NULL);
  glCompileShader(fragmentShader);

  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, sizeof(infoLog), NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }

  glUseProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

void Model::setBuffers() {
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(glm::vec3), &data.vertices[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

  glGenBuffers(1, &nbo);
  glBindBuffer(GL_ARRAY_BUFFER, nbo);
  glBufferData(GL_ARRAY_BUFFER, data.normals.size() * sizeof(glm::vec3), &data.normals[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

  glGenBuffers(1, &mbo);
  glBindBuffer(GL_ARRAY_BUFFER, mbo);
  glBufferData(GL_ARRAY_BUFFER, data.materialIds.size() * sizeof(unsigned int), &data.materialIds[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, (void *)0);

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), &data.indices[0],
                GL_STATIC_DRAW);

  glGenBuffers(1, &ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(Material) * data.materials.size(), &data.materials[0], GL_STATIC_DRAW);
}

void Model::draw(const glm::mat4 &view, const glm::mat4 &projection) {
  glUseProgram(shaderProgram);
  glBindVertexArray(vao);

  glUniformMatrix4fv(0, 1, GL_FALSE, &view[0][0]);
  glUniformMatrix4fv(1, 1, GL_FALSE, &projection[0][0]);

  GLuint location;
  for (int i = 0; i < data.materials.size(); i++) {
    std::string iStr;

    iStr = "materials[" + std::to_string(i) + "].Ka";
    location = glGetUniformLocation(shaderProgram, iStr.c_str());
    glUniform3fv(location, 1, &data.materials[i].Ka[0]);

    iStr = "materials[" + std::to_string(i) + "].Kd";
    location = glGetUniformLocation(shaderProgram, iStr.c_str());
    glUniform3fv(location, 1, &data.materials[i].Kd[0]);

    iStr = "materials[" + std::to_string(i) + "].Ks";
    location = glGetUniformLocation(shaderProgram, iStr.c_str());
    glUniform3fv(location, 1, &data.materials[i].Ks[0]);

    iStr = "materials[" + std::to_string(i) + "].Ns";
    location = glGetUniformLocation(shaderProgram, iStr.c_str());
    glUniform1fv(location, 1, &data.materials[i].Ns);

    iStr = "materials[" + std::to_string(i) + "].Ni";
    location = glGetUniformLocation(shaderProgram, iStr.c_str());
    glUniform1fv(location, 1, &data.materials[i].Ni);

    iStr = "materials[" + std::to_string(i) + "].d";
    location = glGetUniformLocation(shaderProgram, iStr.c_str());
    glUniform1fv(location, 1, &data.materials[i].d);
  }

  glPrimitiveRestartIndex(UINT32_MAX);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glLineWidth(1);

  
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);

  glFrontFace(GL_CCW);
  glDrawElements(GL_TRIANGLE_FAN, data.indices.size(), GL_UNSIGNED_INT, 0);

  glFrontFace(GL_CW);
  glDrawElements(GL_TRIANGLE_FAN, data.indices.size(), GL_UNSIGNED_INT, 0);
}

unsigned int Model::shaderProgram = 0;