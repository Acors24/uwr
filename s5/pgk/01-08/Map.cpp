#include "Map.hpp"
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <epoxy/gl_generated.h>
#include <filesystem>
#include <fstream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <ios>
#include <iostream>
#include <string>

void parseFilename(const std::string &filename, int &lon, int &lat) {
  lon = std::stof(filename.substr(4, 3));
  lat = std::stof(filename.substr(1, 2));

  if (filename[0] == 'S')
    lat *= -1;

  if (filename[3] == 'W')
    lon *= -1;
}

void Map::create(const std::string &dir_name, int x1, int x2, int y1, int y2) {
  init();
  for (const auto &entry : std::filesystem::directory_iterator(dir_name)) {
    const std::string filename = entry.path().filename().string();

    int longitude = 0, latitude = 0;
    parseFilename(filename, longitude, latitude);
    if (filename.rfind(".hgt") != -1 && longitude >= x1 && longitude < x2 && latitude >= y1 && latitude < y2)
      tileAmount++;
  }

  std::cout << "Kafelków: " << tileAmount << std::endl;
  // 1 000 000 000

  heights = new float *[tileAmount];
  for (int i = 0; i < tileAmount; i++)
    heights[i] = new float[1201 * 1201];

  longitudes = new float[tileAmount];
  latitudes = new float[tileAmount];

  int i = 0;
  for (const auto &entry : std::filesystem::directory_iterator(dir_name)) {
    const std::string filename = entry.path().filename().string();

    if (filename.rfind(".hgt") == -1)
      continue;

    int longitude = 0, latitude = 0;
    parseFilename(filename, longitude, latitude);
    if (longitude < x1 || longitude >= x2 || latitude < y1 || latitude >= y2)
      continue;

    longitudes[i] = longitude;
    latitudes[i] = latitude;

    min_lon = std::min(min_lon, (float)longitude);
    max_lon = std::max(max_lon, (float)longitude + 1);
    min_lat = std::min(min_lat, (float)latitude);
    max_lat = std::max(max_lat, (float)latitude + 1);

    std::ifstream in(entry.path().string(), std::ios::binary);
    unsigned char buf[2];
    for (int h = 0; h < 1201 * 1201; h++) {
      in.read(reinterpret_cast<char *>(buf), sizeof(buf));
      heights[i][h] = (short)((buf[0] << 8) | buf[1]);
    }

    i++;
  }

  setBuffers();
}

void Map::destroy() {
  for (int i = 0; i < tileAmount; i++)
    delete heights[i];
  delete heights;

  delete longitudes;
  delete latitudes;
}

static void initIndices() {
  int i = 0;
  for (int y = 0; y < 1201 - 1; y++) {
    for (int x = 0; x < 1201; x++) {
      Map::indices0[i++] = x + y * 1201;
      Map::indices0[i++] = x + (y + 1) * 1201;
    }
    Map::indices0[i++] = UINT32_MAX;
  }

  i = 0;
  for (int y = 0; y < 1201 - 1; y += 2) {
    for (int x = 0; x < 1201; x += 2) {
      Map::indices1[i++] = x + y * 1201;
      Map::indices1[i++] = x + (y + 2) * 1201;
    }
    Map::indices1[i++] = UINT32_MAX;
  }

  i = 0;
  for (int y = 0; y < 1201 - 1; y += 3) {
    for (int x = 0; x < 1201; x += 3) {
      Map::indices2[i++] = x + y * 1201;
      Map::indices2[i++] = x + (y + 3) * 1201;
    }
    Map::indices2[i++] = UINT32_MAX;
  }

  i = 0;
  for (int y = 0; y < 1201 - 1; y += 4) {
    for (int x = 0; x < 1201; x += 4) {
      Map::indices3[i++] = x + y * 1201;
      Map::indices3[i++] = x + (y + 4) * 1201;
    }
    Map::indices3[i++] = UINT32_MAX;
  }

  i = 0;
  for (int y = 0; y < 1201 - 1; y += 6) {
    for (int x = 0; x < 1201; x += 6) {
      Map::indices4[i++] = x + y * 1201;
      Map::indices4[i++] = x + (y + 6) * 1201;
    }
    Map::indices4[i++] = UINT32_MAX;
  }

  i = 0;
  for (int y = 0; y < 1201 - 1; y += 12) {
    for (int x = 0; x < 1201; x += 12) {
      Map::indices5[i++] = x + y * 1201;
      Map::indices5[i++] = x + (y + 12) * 1201;
    }
    Map::indices5[i++] = UINT32_MAX;
  }

  i = 0;
  for (int y = 0; y < 1201 - 1; y += 24) {
    for (int x = 0; x < 1201; x += 24) {
      Map::indices6[i++] = x + y * 1201;
      Map::indices6[i++] = x + (y + 24) * 1201;
    }
    Map::indices6[i++] = UINT32_MAX;
  }

  i = 0;
  for (int y = 0; y < 1201 - 1; y += 120) {
    for (int x = 0; x < 1201; x += 120) {
      Map::indices7[i++] = x + y * 1201;
      Map::indices7[i++] = x + (y + 120) * 1201;
    }
    Map::indices7[i++] = UINT32_MAX;
  }
}

void Map::init() {
  static bool _init = false;

  if (!_init) {
    initIndices();
    setShaders();

    _init = true;
  }
}

void Map::setShaders() {
  const char *vertexShaderSource = R"END(
         #version 330
         #extension GL_ARB_explicit_uniform_location : require
         #extension GL_ARB_shading_language_420pack : require

         layout(location = 0) in float inHeight;
         
         layout(location = 0) uniform mat4 view;
         layout(location = 1) uniform mat4 projection;
         layout(location = 2) uniform float longitude;
         layout(location = 3) uniform float latitude;
         layout(location = 4) uniform float flat_;

         out float ht;

         void main(void) {
          float row = gl_VertexID / 1201;
          float col = gl_VertexID % 1201;

          float row_norm = row / 1200.0f;
          float col_norm = col / 1200.0f;

          float x = 0.0f, y = 0.0f, z = 0.0f;
          float radius_at = 6378.0f + inHeight / 1000.0f;

          float longitude = longitude + col_norm;
          float latitude = latitude - row_norm;
          if (flat_ > 0.5f) {
            x = longitude;
            y = degrees(tan(radians(latitude)));
            z = 0.0f;
          } else {
            x = radius_at * sin(radians(latitude - 90.0f)) * cos(radians(longitude));
            y = radius_at * cos(radians(latitude - 90.0f));
            z = radius_at * sin(radians(latitude - 90.0f)) * sin(-radians(longitude));
          }

          gl_Position = projection * view * vec4(x, y, z, 1.0f);

          ht = inHeight;
         }
      )END";
  const char *fs = R"END(
         #version 330
         #extension GL_ARB_explicit_uniform_location : require

         in float ht;

         out vec4 color;

         void main(void) {
            if      (ht <= 0)    color = vec4(0.,       0.,                          1, 1.0f); //->blue
            else if (ht < 500)   color = vec4(0.,       ht/500,                      0, 1.0f); //->green
            else if (ht < 1000)  color = vec4(ht/500-1, 1.,                          0, 1.0f); //->yellow
            else if (ht < 2000)  color = vec4(1.,       2.-ht/1000,                  0, 1.0f); //->red
            else                 color = vec4(1.,       (ht-2000)/7000, (ht-2000)/7000, 1.0f); //->white
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

void Map::setBuffers() {
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(heights[0]), heights[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, (void *)0);

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
}

void Map::drawAll(const glm::mat4 &view, const glm::mat4 &projection, int &rendered) {
  glUseProgram(shaderProgram);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glUniformMatrix4fv(0, 1, GL_FALSE, &view[0][0]);
  glUniformMatrix4fv(1, 1, GL_FALSE, &projection[0][0]);
  glUniform1fv(4, 1, &flat);

  glPrimitiveRestartIndex(UINT32_MAX);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glLineWidth(1);

  int l = 0;
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  switch (lod) {
  case NATIVE:
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices0), indices0,
                 GL_STATIC_DRAW);
    l = 0;
    break;
  case HIGH:
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices1), indices1,
                 GL_STATIC_DRAW);
    l = 1;
    break;
  case MEDIUM:
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2,
                 GL_STATIC_DRAW);
    l = 2;
    break;
  case LOW:
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices3), indices3,
                 GL_STATIC_DRAW);
    l = 3;
    break;
  case AWFUL:
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices4), indices4,
                 GL_STATIC_DRAW);
    l = 4;
    break;
  case POTATO:
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices5), indices5,
                 GL_STATIC_DRAW);
    l = 5;
    break;
  case NONE:
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices6), indices6,
                 GL_STATIC_DRAW);
    l = 6;
    break;
  case USOS:
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices7), indices7,
                 GL_STATIC_DRAW);
    l = 7;
    break;

  // AUTO
  default:
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices0), indices0,
                 GL_STATIC_DRAW);
    l = 0;
  }

  int tileSizes[] = { 1200, 600, 400, 300, 200, 100, 50, 10};
  int tileSize = tileSizes[l];

  for (int i = 0; i < tileAmount; i++) {
    glUniform1fv(2, 1, &longitudes[i]);
    glUniform1fv(3, 1, &latitudes[i]);

    glBufferData(GL_ARRAY_BUFFER, 1201 * 1201 * sizeof(float), heights[i],
                 GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLE_STRIP, indexAmounts[l], GL_UNSIGNED_INT, 0);
  }

  rendered = tileSize * tileSize * 2 * tileAmount;
}

void Map::toggleFlat() { flat = 1.0 - flat; }

bool Map::isFlat() { return flat > 0.5f; }

unsigned int Map::vao = 0;
unsigned int Map::vbo = 0;
unsigned int Map::ebo = 0;
unsigned int Map::shaderProgram = 0;
unsigned int Map::lod = 0;
float Map::flat = 1.0f;
int Map::tileAmount = 0;
float Map::min_lon = MAXFLOAT, Map::max_lon = -MAXFLOAT,
      Map::min_lat = MAXFLOAT, Map::max_lat = -MAXFLOAT;

float **Map::heights;
float *Map::longitudes;
float *Map::latitudes;

constexpr unsigned int Map::indexAmounts[];
unsigned int Map::indices0[] = {};
unsigned int Map::indices1[] = {};
unsigned int Map::indices2[] = {};
unsigned int Map::indices3[] = {};
unsigned int Map::indices4[] = {};
unsigned int Map::indices5[] = {};
unsigned int Map::indices6[] = {};
unsigned int Map::indices7[] = {};