#ifndef BOX_HPP
#define BOX_HPP

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "AGL3Drawable.hpp"

#include "stb_image.h"

class Box : AGLDrawable
{
public:
    static const float vertices[6 * 6 * 3 + 6 * 6 * 2];
    const float scale;

    glm::vec3 position;

    Box(float scale);

    void setBuffers();
    void setShaders();

    glm::mat4 getModelMatrix() const;

    void draw(const glm::mat4 &view, const glm::mat4 &projection, float collapse_time, bool fog);
};

Box::Box(float scale) : position({0.5f, 0.5f, 0.5f}), scale(scale)
{
    setShaders();
    setBuffers();
}

void Box::setShaders()
{
    compileShaders(R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require
         #extension GL_ARB_shading_language_420pack : require
         layout(location = 0) in vec3 vpos;
         layout(location = 2) in vec2 aTexCoord;
         layout(location = 1) uniform mat4 model;
         layout(location = 2) uniform mat4 view;
         layout(location = 3) uniform mat4 projection;

         out vec4 vcolor;
         out vec2 TexCoord;
         out float dist;

         void main(void) {
            gl_Position = view * model * vec4(vpos, 1);
            vcolor = vec4(vpos, 1.0f);
            dist = length(gl_Position.xyz);
            TexCoord = aTexCoord;
            gl_Position = projection * gl_Position;
         }

      )END",
                   R"END(

        #version 330 
        #extension GL_ARB_explicit_uniform_location : require

        in vec4 vcolor;
        in vec2 TexCoord;
        in float dist;
        layout(location = 4) uniform float time;
        layout(location = 5) uniform float fog;
        layout(location = 6) uniform float c_time;
        out vec4 color;
        uniform sampler2D ourTexture;

        float rand(float n){return fract(sin(n) * 43758.5453123);}

        float noise(float p){
            float fl = floor(p);
            float fc = fract(p);
            return mix(rand(fl), rand(fl + 1.0), fc);
        }

        void main(void) {
            color = texture(ourTexture, TexCoord);

            if (fog > 0.5f)
            {
                color = mix(color, vec4(0.0f), min(1, dist * dist * 5));
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

void Box::setBuffers()
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
        std::cout << "Failed to load the box texture." << std::endl;
    }
    stbi_image_free(data);

    bindBuffers();

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                 // attribute 0, must match the layout in the shader.
        3,                 // size
        GL_FLOAT,          // type
        GL_FALSE,          // normalized?
        5 * sizeof(float), // 24,             // stride
        (void *)0          // array buffer offset
    );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(float),
        (void *)(3 * sizeof(float)));
}

void Box::draw(const glm::mat4 &view, const glm::mat4 &projection, float collapse_time, bool fog)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(scale));

    bindProgram();
    bindBuffers();

    float time = glfwGetTime();

    glUniformMatrix4fv(1, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(2, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(3, 1, GL_FALSE, &projection[0][0]);
    glUniform1fv(4, 1, &time);
    float _fog = fog ? 1.0f : 0.0f;
    glUniform1fv(5, 1, &_fog);
    glUniform1fv(6, 1, &collapse_time);

    glLineWidth(1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_FRONT);

    glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices));

    glDisable(GL_CULL_FACE);
}

glm::mat4 Box::getModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(scale));

    return model;
}

const float Box::vertices[] = {
    -1.0f, -1.0f, -1.0f, -10.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, -10.0f, -10.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 1.0f,

    1.0f, 1.0f, -1.0f, 1.0f, -10.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, -10.0f, -10.0f,

    -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 1.0f, 0.0f,

    -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,

    -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,

    1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 0.0f,

    -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

    1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.0f, 1.0f,

    1.0f, -1.0f, -1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 1.0f, 1.0f,

    1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 0.0f, 0.0f,

    -1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 1.0f, 0.0f,

    1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 1.0f, 0.0f, 1.0f};

#endif