#include "Bubble.hpp"
#include <GL/gl.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <glm/gtx/norm.hpp>
#include <iostream>

static void initMesh()
{
    int i = 0;
    const int RES = Bubble::RESOLUTION;
    float x, y, z, hTheta, vTheta, vTheta2, radius;
    for (int j = 0; j < RES; j++)
    {
        vTheta = j * M_PI / RES;
        vTheta2 = (j + 1) * M_PI / RES;

        for (int k = 0; k <= RES; k++)
        {
            hTheta = k * 2 * M_PI / RES;
            radius = std::sin(vTheta);

            x = std::cos(hTheta) * radius;
            y = std::cos(vTheta);
            z = std::sin(hTheta) * radius;

            Bubble::vertices[i++] = glm::vec3(x, y, z);

            radius = std::sin(vTheta2);

            x = std::cos(hTheta) * radius;
            y = std::cos(vTheta2);
            z = std::sin(hTheta) * radius;

            Bubble::vertices[i++] = glm::vec3(x, y, z);
        }
    }
}

static void initBuffers()
{
    glGenBuffers(1, &Bubble::vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Bubble::vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Bubble::vertices), Bubble::vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &Bubble::positionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Bubble::positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, Bubble::AMOUNT * 4 * sizeof(float), NULL, GL_STREAM_DRAW);

    glGenBuffers(1, &Bubble::colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Bubble::colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, Bubble::AMOUNT * 4 * sizeof(float), NULL, GL_STREAM_DRAW);

    glGenBuffers(1, &Bubble::propertyBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Bubble::propertyBuffer);
    glBufferData(GL_ARRAY_BUFFER, Bubble::AMOUNT * 4 * sizeof(float), NULL, GL_STREAM_DRAW);
}

void Bubble::init()
{
    static bool _init = false;
    
    if (!_init)
    {
        initMesh();
        initBuffers();
        difficulty = 1;
        for (int i = 0; i < AMOUNT; i++)
            initInstance(i);
        setShaders();
        setBuffers();

        _init = true;
    }
}

void Bubble::setShaders()
{
    const char *vertexShaderSource = R"END(
         #version 330
         #extension GL_ARB_explicit_uniform_location : require
         #extension GL_ARB_shading_language_420pack : require

         layout(location = 0) in vec3 vertexPos;
         layout(location = 1) in vec4 objectPos;
         layout(location = 2) in vec4 objectColor;
         layout(location = 3) in vec4 objectProperties;
         
         layout(location = 0) uniform mat4 view;
         layout(location = 1) uniform mat4 projection;
         layout(location = 2) uniform vec3 cameraPos;

         out vec3 N;
         out vec3 L;
         out vec3 V;
         out vec4 objColor;
         out vec3 fragPos;
         flat out float isSpecial;
         out float radius;

         void main(void) {
            mat4 model = mat4(1.0f);
            model[3] = vec4(objectPos.xyz, 1.0f);
            gl_Position = model * vec4(vertexPos.xyz * objectPos.w, 1.0f);
            fragPos = gl_Position.xyz;
            objColor = objectColor;
            isSpecial = objectProperties.w;
            radius = objectPos.w;

            N = mat3(transpose(inverse(model))) * normalize(vertexPos);
            L = vec3(1.0f, 1.0f, 1.0f);
            V = cameraPos - gl_Position.xyz;

            gl_Position = projection * view * gl_Position;
         }
      )END";
      const char *fs = R"END(
         #version 330
         #extension GL_ARB_explicit_uniform_location : require

         layout(location = 2) uniform vec3 cameraPos;
         layout(location = 3) uniform float time;
         layout(location = 4) uniform vec3 playerPos;
         layout(location = 5) uniform float fog;
         layout(location = 6) uniform vec3 lightPositions[10];
         layout(location = 16) uniform vec4 bubblePositions[1000];
         layout(location = 3016) uniform vec4 bubbleColors[1000];
         
         in vec3 N;
         in vec3 L;
         in vec3 V;
         in vec4 objColor;
         in vec3 fragPos;
         flat in float isSpecial;
         in float radius;
         out vec4 color;

         void main(void) {
            vec3 N = normalize(N);
            vec3 L = normalize(L);
            vec3 V = normalize(V);

            float time = time * 2;

            vec3 is = vec3(1.0f, 1.0f, 1.0f);
            vec3 id = vec3(1.0f, 1.0f, 1.0f);
            vec3 ia = vec3(0.2f);

            vec3 ks = vec3(1.0f);
            vec3 kd = objColor.xyz;
            vec3 ka = vec3(1.0f);
            float alpha = 64.0f;

            vec3 R = 2.0f * dot(L, N) * N - L;
            vec3 I = ka * ia + kd * max(dot(L, N), 0) * id + ks * pow(max(dot(R, V), 0), alpha) * is;

            // Special
            for (int i = 0; i < 10; i++)
            {
                vec3 L = lightPositions[i] - fragPos;
                float distanceToLightSquared = dot(L, L);
                L = normalize(L);
                vec3 R = 2.0f * dot(L, N) * N - L;
                float factor = max(0.0f, sin(time)) * 2.0f;
                vec3 term = kd * max(dot(L, N), 0) * id * factor + ks * pow(max(dot(R, V), 0), alpha) * is * factor;
                term /= distanceToLightSquared + 1.0f;
                I += term;
            }

            // Player
            {
                vec3 L = playerPos - fragPos;
                float distanceToLightSquared = dot(L, L);
                L = normalize(L);
                vec3 R = 2.0f * dot(L, N) * N - L;
                vec3 term = kd * max(dot(L, N), 0) * id + ks * pow(max(dot(R, V), 0), alpha) * is;
                term /= distanceToLightSquared + 1.0f;
                I += term * 2.0f;

                color = vec4(I, objColor.w);

                // float a = R.x * R.x * R.y + R.z * R.z * R.y;
                // float b = 2 * fragPos.x * R.x * R.y + R.x * R.x * fragPos.y + 2 * fragPos.z * R.z * R.y + R.z * R.z * fragPos.y;
                // float c = fragPos.x * fragPos.x * R.y + 2 * fragPos.x * R.x * fragPos.y + fragPos.z * fragPos.z * R.y + 2 * fragPos.z * R.z * fragPos.y;
                // float d = fragPos.x * fragPos.x * fragPos.y + fragPos.z * fragPos.z * fragPos.y - 2;

                // float delta0 = b*b - 3*a*c;
                // float delta1 = 2*b*b*b - 9*a*b*c + 27*a*a*d;

                // float C = pow((delta1 + pow(delta1 * delta1 - 4 * delta0 * delta0 * delta0, 0.5f)) / 2, 1.0f / 3.0f);
                // if (C == 0)
                //     C = pow((delta1 - pow(delta1 * delta1 - 4 * delta0 * delta0 * delta0, 0.5f)) / 2, 1.0f / 3.0f);

                // if (C != 0)
                // {
                //     color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
                // }
                // // float t = -1 / (3 * a) * (b + C + delta0 / C);
            }

            // Ray tracing
            {
                vec3 center = fragPos - V * radius;
                vec3 R = 2.0f * dot(V, N) * N - V;
                float closest = 100.0f;

                for (int i = 0; i < 50; i++)
                {
                    vec3 bubbleCenter = bubblePositions[i].xyz;
                    if (dot(bubbleCenter - center, bubbleCenter - center) < 0.01f) continue;

                    vec3 ocVec = fragPos - bubbleCenter;
                    float ROcVecDot = dot(R, ocVec);
                    float nabla = pow(ROcVecDot, 2) - (dot(ocVec, ocVec) - pow(bubblePositions[i].w, 2));

                    if (nabla >= 0)
                    {
                        float d = -ROcVecDot - sqrt(nabla);
                        if (d < closest)
                        {
                            color.xyz = mix(bubbleColors[i].xyz, color.xyz, clamp(d / 1.0f, 0.0f, 1.0f));
                            closest = d;
                        }
                    }
                }
            }

            if (isSpecial > 0.5f)
            {
                color = mix(color, vec4(1.0f, 1.0f, 1.0f, color.w), max(0.0f, sin(time)));
            }

            if (fog > 0.5f)
            {
                vec3 distanceVec = fragPos - cameraPos;
                float distanceToCamera = length(distanceVec);
                vec4 fogColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
                color = mix(color, fogColor, min(1.0f, exp(distanceToCamera * 0.04f) - 1.0f));
            }
         }
      )END";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    glShaderSource(fragmentShader, 1, &fs, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glUseProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
}

void Bubble::setBuffers()
{
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glVertexAttribPointer(
        1,
        4,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glVertexAttribPointer(
        2,
        4,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, propertyBuffer);
    glVertexAttribPointer(
        3,
        4,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );

    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);

}

void Bubble::drawAll(const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &cameraPos, const glm::vec3 &playerPos, bool fog)
{
    float time = glfwGetTime();
    float _fog = fog;
    glm::vec3 lightPositions[10];
    for (int i = 0; i < 10; i++)
        lightPositions[i] = positions[i];

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    glUniformMatrix4fv(0, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(1, 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(2, 1, &cameraPos[0]);
    glUniform1fv(3, 1, &time);
    glUniform3fv(4, 1, &playerPos[0]);
    glUniform1fv(5, 1, &_fog);
    glUniform3fv(6, 10, &lightPositions[0][0]);
    glUniform4fv(16, Bubble::AMOUNT, &positions[0][0]);
    glUniform4fv(3016, Bubble::AMOUNT, &colors[0][0]);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glLineWidth(1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glFrontFace(GL_CW);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, sizeof(vertices) / sizeof(glm::vec3), AMOUNT);
    
    glFrontFace(GL_CCW);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, sizeof(vertices) / sizeof(glm::vec3), AMOUNT);

    glDisable(GL_CULL_FACE);
}

void Bubble::updateAll(float deltaTime, const glm::vec3 &cameraPos)
{
    for (int i = 0; i < AMOUNT; i++)
    {
        float &radius = positions[i].w;
        positions[i].y += properties[i][RISINGSPEED] * deltaTime;
        radius += properties[i][INFLATINGSPEED] * deltaTime;

        if (radius > 1.5f * properties[i][MAXRADIUS])
        {
            initInstance(i);
        }
        else if (radius > properties[i][MAXRADIUS])
        {
            radius *= 1.5f;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, AMOUNT * 4 * sizeof(float), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, AMOUNT * 4 * sizeof(float), positions);

    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, AMOUNT * 4 * sizeof(float), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, AMOUNT * 4 * sizeof(float), colors);

    glBindBuffer(GL_ARRAY_BUFFER, propertyBuffer);
    glBufferData(GL_ARRAY_BUFFER, AMOUNT * 4 * sizeof(float), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, AMOUNT * 4 * sizeof(float), properties);
}

void Bubble::initInstance(int id)
{
    const float spawnAreaRadius = 18.0f;
    const float spawnRadiusOffset = 0.8f;
    float spawnRadius = rand() / (double)RAND_MAX * spawnAreaRadius + spawnRadiusOffset;
    float spawnDegree = rand() / (double)RAND_MAX * 2.0f * M_PI;
    float x = std::cos(spawnDegree) * spawnRadius;
    float y = rand() / (double)RAND_MAX - 1.0f;
    float z = std::sin(spawnDegree) * spawnRadius;

    float risingSpeed = difficulty * 0.1f * rand() / (double)RAND_MAX + 0.08f;
    float inflatingSpeed = difficulty * 0.01f * rand() / (double)RAND_MAX + 0.003f;
    float maxRadius = 0.3 * rand() / (double)RAND_MAX + 0.1f;
    float radius = 0.0f;
    positions[id] = glm::vec4(x, y, z, radius);
    properties[id][MAXRADIUS] = maxRadius;
    properties[id][RISINGSPEED] = risingSpeed;
    properties[id][INFLATINGSPEED] = inflatingSpeed;
    properties[id][SPECIAL] = id < 10;

    colors[id] = {0.0f, 0.0f, 0.0f, 0.3f * rand() / RAND_MAX + 0.2f};
    int full = rand() % 3, other;
    do
    {
        other = rand() % 3;
    } while (other == full);
    colors[id][full] = 1.0f;
    colors[id][other] = rand() / (double)RAND_MAX;
}

glm::vec3 Bubble::vertices[];
glm::vec4 Bubble::positions[];
glm::vec4 Bubble::colors[];
glm::vec4 Bubble::properties[];

unsigned int Bubble::VAO;
GLuint Bubble::vertexBuffer, Bubble::positionBuffer, Bubble::colorBuffer, Bubble::propertyBuffer;
unsigned int Bubble::shaderProgram;
int Bubble::difficulty;