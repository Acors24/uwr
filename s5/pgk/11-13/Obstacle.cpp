#include "Obstacle.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <GLFW/glfw3.h>

#include "stb_image.h"

Obstacle::Obstacle(float x, float y, float z)
{
    position = {x, y, z};
    rotation = {
        M_2_PI * rand() / RAND_MAX,
        M_2_PI * rand() / RAND_MAX,
        M_2_PI * rand() / RAND_MAX,
        0};
    color = {x, y, z, 1};

    setShaders();
    setBuffers();
}

void Obstacle::setPosition(float x, float y, float z)
{
    position = {x, y, z};
    color = {x, y, z, 1};
}

void Obstacle::setShaders()
{
    compileShaders(R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require
         #extension GL_ARB_shading_language_420pack : require
         layout(location = 0) in vec4 vpos;
         layout(location = 3) uniform mat4 model;
         layout(location = 4) uniform mat4 view;
         layout(location = 5) uniform mat4 projection;

         out float dist;

         void main(void) {
            gl_Position = view * model * vpos;
            dist = length(gl_Position.xyz);
            gl_Position = projection * gl_Position;
         }

      )END",
                   R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require
         layout(location = 0) uniform float colliding;
         layout(location = 1) uniform float isOutline;
         layout(location = 2) uniform vec4 vcolor;
         layout(location = 6) uniform float time;
         layout(location = 7) uniform float fog;
         layout(location = 8) uniform vec3 pos;
         layout(location = 9) uniform float c_time;
         in float dist;
         out vec4 color;

        float rand(float n){return fract(sin(n) * 43758.5453123);}

        float noise(float p){
            float fl = floor(p);
            float fc = fract(p);
            return mix(rand(fl), rand(fl + 1.0), fc);
        }

         void main(void) {
            color.x = vcolor.x + sin(pos.x + time * 2 + 0) * 0.05;
            color.y = vcolor.y + sin(pos.y + time * 2 + 2) * 0.05;
            color.z = vcolor.z + sin(pos.z + time * 2 + 4) * 0.05;
            color = mix(color, vec4(1) - color, isOutline);

            float pulse = exp(-mod(2 * time + 2 * pos.x + 2 * pos.y + 2 * pos.z, 4));
            color += pulse * vec4(pos, 1.0f);

            if (fog > 0.5f)
            {
                color = mix(color, vec4(0.0f), min(1, dist * dist * 20));
            }

            if (colliding > 0.5f)
            {
                // float red = sin(10 * time + 0.01 * gl_FragCoord.x + 0.01 * gl_FragCoord.y + 0.01 * gl_FragCoord.z) / 4 + 0.25;
                float len = length(gl_FragCoord);
                float red = sin(time * 5 + 0.05 * gl_FragCoord.x * gl_FragCoord.y) / 2;
                float factor = max(0, (sin(5 * time) + sin(5 * 3 * time)) / 1.5);
                color = mix(color, vec4(red, 0, 0, 1), factor);
            }

            if (pow(c_time, 10) > rand(time * gl_FragCoord.x + time * gl_FragCoord.y))
            {
                float factor = (c_time + 1) * (c_time + 1);
                vec4 new_color = vec4(rand(gl_FragCoord.x + factor), rand(gl_FragCoord.y + factor), rand(gl_FragCoord.x * gl_FragCoord.y + factor), 1.0f);
                color = mix(color, new_color, min(1, c_time));
                // discard;
            }
         } 

      )END");
}

glm::mat4 Obstacle::getModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0));
    model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
    model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(scale));

    return model;
}

void Obstacle::setBuffers()
{
    bindBuffers();

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,        // attribute 0, must match the layout in the shader.
        4,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // 24,             // stride
        (void *)0 // array buffer offset
    );
}

void Obstacle::draw(const glm::mat4 &view, const glm::mat4 &projection, float collapse_time, bool hollow, bool fog)
{
    glm::mat4 model = getModelMatrix();

    float isOutline = 0, time = glfwGetTime();
    bindProgram();
    bindBuffers();
    glUniform1fv(0, 1, &colliding);
    glUniform1fv(1, 1, &isOutline);
    glUniform4fv(2, 1, &color[0]);
    glUniformMatrix4fv(3, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(4, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(5, 1, GL_FALSE, &projection[0][0]);
    glUniform1fv(6, 1, &time);
    float _fog = fog ? 1.0f : 0.0f;
    glUniform1fv(7, 1, &_fog);
    glUniform3fv(8, 1, &position[0]);
    glUniform1fv(9, 1, &collapse_time);

    // glPolygonMode(GL_FRONT_AND_BACK, hollow ? GL_LINE : GL_FILL);

    // glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
    glLineWidth(1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(hollow ? GL_POINTS : GL_TRIANGLE_STRIP, 0, 6);

    if (!hollow)
    {
        isOutline = 1;
        glUniform1fv(1, 1, &isOutline);
        glLineWidth(2);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
    }
}

const float Obstacle::vertices[] = {
    0.0f, 1.0f, -1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f, 1.0f,
    0.0f, 1.0f, -1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f};

float Obstacle::scale = 0.1f;

Goal::Goal(float x, float y, float z) : Obstacle(x, y, z)
{
    position = {x, y, z};
    rotation = {
        M_2_PI * rand() / RAND_MAX,
        M_2_PI * rand() / RAND_MAX,
        M_2_PI * rand() / RAND_MAX,
        0};
    color = {x, y, z, 1};

    setShaders();
    setBuffers();
}

void Goal::setShaders()
{
    compileShaders(R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require
         #extension GL_ARB_shading_language_420pack : require
         layout(location = 0) in vec4 vpos;
         layout(location = 2) in vec2 aTexCoord;
         layout(location = 3) uniform mat4 model;
         layout(location = 4) uniform mat4 view;
         layout(location = 5) uniform mat4 projection;

         out float dist;
         out vec2 TexCoord;

         void main(void) {
            gl_Position = view * model * vpos;
            dist = length(gl_Position.xyz);
            gl_Position = projection * gl_Position;
         }

      )END",
                   R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require
         layout(location = 0) uniform float colliding;
         layout(location = 1) uniform float isOutline;
         layout(location = 2) uniform vec4 vcolor;
         layout(location = 6) uniform float time;
         layout(location = 7) uniform float fog;
         layout(location = 8) uniform vec3 pos;
         layout(location = 9) uniform float c_time;
         in float dist;
         in vec2 TexCoord;
         out vec4 color;
         uniform sampler2D ourTexture;

        float rand(float n){return fract(sin(n) * 43758.5453123);}

        float noise(float p){
            float fl = floor(p);
            float fc = fract(p);
            return mix(rand(fl), rand(fl + 1.0), fc);
        }

         void main(void) {
            color = vec4(1.0f);

            float pulse = exp(-mod(2 * time + 2 * pos.x + 2 * pos.y + 2 * pos.z, 4));
            color += pulse * vec4(pos, 1.0f);

            if (fog > 0.5f)
            {
                color = mix(color, vec4(0.0f), min(1, dist * dist * 20));
            }

            if (pow(c_time, 10) > rand(time * gl_FragCoord.x + time * gl_FragCoord.y))
            {
                float factor = (c_time + 1) * (c_time + 1);
                vec4 new_color = vec4(rand(gl_FragCoord.x + factor), rand(gl_FragCoord.y + factor), rand(gl_FragCoord.x * gl_FragCoord.y + factor), 1.0f);
                color = mix(color, new_color, min(1, c_time));
            }
         } 

      )END");
}

void Goal::setBuffers()
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("obamium.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load the goal texture." << std::endl;
    }
    stbi_image_free(data);

    bindBuffers();

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,        // attribute 0, must match the layout in the shader.
        4,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        6 * sizeof(float),        // 24,             // stride
        (void *)0 // array buffer offset
    );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void *)(4 * sizeof(float)));
}

void Goal::draw(const glm::mat4 &view, const glm::mat4 &projection, float collapse_time, bool hollow, bool fog)
{
    glm::mat4 model = getModelMatrix();

    float isOutline = 0, time = glfwGetTime();

    bindProgram();
    bindBuffers();

    glUniform1fv(0, 1, &colliding);
    glUniform1fv(1, 1, &isOutline);
    glUniform4fv(2, 1, &color[0]);
    glUniformMatrix4fv(3, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(4, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(5, 1, GL_FALSE, &projection[0][0]);
    glUniform1fv(6, 1, &time);
    float _fog = fog ? 1.0f : 0.0f;
    glUniform1fv(7, 1, &_fog);
    glUniform3fv(8, 1, &position[0]);
    glUniform1fv(9, 1, &collapse_time);

    glLineWidth(1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(hollow ? GL_POINTS : GL_TRIANGLE_STRIP, 0, 6);

    if (!hollow)
    {
        isOutline = 1;
        glUniform1fv(1, 1, &isOutline);
        glLineWidth(2);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
    }
}

const float Goal::vertices[] = {
    0.0f, 1.0f, -1.0f, 1.0f,  0.19597f, 0.48687f,
    0.0f, 0.0f, 1.0f, 1.0f,   0.03639f, 0.06792f,
    1.0f, -1.0f, -1.0f, 1.0f, 0.49041f, 0.20006f,
    -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
