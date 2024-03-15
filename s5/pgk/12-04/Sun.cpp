#include "Sun.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

static int initMesh()
{
    int i = 0;
    const int RES = Sun::RESOLUTION;
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

            Sun::vertices[i++] = glm::vec3(x, y, z);
            
            radius = std::sin(vTheta2);

            x = std::cos(hTheta) * radius;
            y = std::cos(vTheta2);
            z = std::sin(hTheta) * radius;

            Sun::vertices[i++] = glm::vec3(x, y, z);
        }
    }

    return 0;
}

Sun::Sun()
{
    static int _init = initMesh();

    float x = 30.0f;
    float y = 30.0f;
    float z = 30.0f;

    position = glm::vec3(x, y, z);
    radius = 0.3f;

    color = glm::vec3(1.0f, 0.7f, 0.4f);

    setShaders();
    setBuffers();
}

void Sun::setShaders()
{
    compileShaders(R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require
         #extension GL_ARB_shading_language_420pack : require
         layout(location = 0) in vec3 vpos;
         layout(location = 0) uniform mat4 model;
         layout(location = 1) uniform mat4 view;
         layout(location = 2) uniform mat4 projection;
         layout(location = 4) uniform vec3 position;

         out vec3 V;
         out vec3 fragPos;

         void main(void) {
            // fragPos = (model * vec4(vpos, 1.0f)).xyz;
            fragPos = vpos;
            gl_Position = projection * view * model * vec4(vpos, 1.0f);
            V = normalize(vec3(-1.0f));
         }

      )END",
                   R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require

         layout(location = 3) uniform vec3 sunColor;

         in vec3 V;
         in vec3 fragPos;
         out vec4 color;

         void main(void) {
            float angle = dot(V, fragPos);
            color = mix(vec4(1.0f), vec4(sunColor, 1.0f), pow(angle, 4));
         } 

      )END");
}

void Sun::setBuffers()
{
    bindBuffers();

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,        // attribute 0, must match the layout in the shader.
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // 24,             // stride
        (void *)0 // array buffer offset
    );
}

glm::mat4 Sun::getModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(radius));

    return model;
}

void Sun::draw(const glm::mat4 &view, const glm::mat4 &projection)
{
    glm::mat4 model = getModelMatrix();

    bindProgram();
    bindBuffers();
    glUniformMatrix4fv(0, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(1, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(2, 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(3, 1, &color[0]);
    glUniform3fv(4, 1, &position[0]);

    glLineWidth(1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(vertices) / sizeof(glm::vec3));
}

glm::vec3 Sun::vertices[];