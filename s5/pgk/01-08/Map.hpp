#ifndef MAP_HPP
#define MAP_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <epoxy/gl.h>
#include <string>

enum LOD {
    NATIVE,
    HIGH,
    MEDIUM,
    LOW,
    AWFUL,
    POTATO,
    NONE,
    USOS,

    AUTO
};

class Map
{
public:
    static float **heights;
    static GLuint vbo;
    static float *longitudes;
    static float *latitudes;

    constexpr static unsigned int indexAmounts[] = {
        (1201 * 2 + 1) * 1200,
        (601 * 2 + 1) * 600,
        (401 * 2 + 1) * 400,
        (301 * 2 + 1) * 300,
        (201 * 2 + 1) * 200,
        (101 * 2 + 1) * 100,
        (51 * 2 + 1) * 50,
        (11 * 2 + 1) * 10,
    };

    static unsigned int indices0[indexAmounts[0]];
    static unsigned int indices1[indexAmounts[1]];
    static unsigned int indices2[indexAmounts[2]];
    static unsigned int indices3[indexAmounts[3]];
    static unsigned int indices4[indexAmounts[4]];
    static unsigned int indices5[indexAmounts[5]];
    static unsigned int indices6[indexAmounts[6]];
    static unsigned int indices7[indexAmounts[7]];

    static unsigned int vao;
    static unsigned int ebo;
    static unsigned int shaderProgram;

    static unsigned int lod;
    static float flat;

    static int tileAmount;

    static float min_lon, max_lon, min_lat, max_lat;

    Map() = delete;

    static void init();

    static void initInstance(int id);

    static void create(const std::string &dirName, int x1, int x2, int y1, int y2);
    static void destroy();

    static void setBuffers();
    static void setShaders();

    glm::mat4 getModelMatrix() const;

    static void drawAll(const glm::mat4 &view, const glm::mat4 &projection, int &rendered);

    static void toggleFlat();

    static bool isFlat();
};

#endif