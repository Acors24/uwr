#include "Volcano.hpp"

Volcano::Volcano() : position({0.0f, 0.0f, 0.0f})
{
    int i = 0;

    auto f = [](float x)
    { return 1.0f / x; };

    auto inside = [](float x)
    { return -1.0f / (x + 2.0f) + 0.5f; };

    auto map = [](float x, float fromA, float fromB, float toA, float toB)
    {
        // [fA; fB] ~~> [tA; tB]
        return (x - fromA) / (fromB - fromA) * (toB - toA) + toA;
    };

    float radius, x, y, z, height, theta, randFactor, randStart;

    // Inside
    for (int _height = 1, diff = 1; _height < 30; _height += diff)
    {
        for (int j = 0; j <= RESOLUTION; j++)
        {
            theta = j * 2 * M_PI / RESOLUTION;

            height = _height / 10.0f;
            radius = inside(height);
            randFactor = map(std::log(height) + 2, 0.0f, std::log(3.0f) + 2, 0.3f, 0.03f);
            randStart = -randFactor * 0.5f;

            x = randFactor * std::rand() / (double)RAND_MAX + randStart + radius * std::cos(theta);
            y = randFactor * std::rand() / (double)RAND_MAX + randStart + height;
            z = randFactor * std::rand() / (double)RAND_MAX + randStart + radius * std::sin(theta);

            vertices[i++] = glm::vec3(x, y, z) * SCALE;

            height = (_height + 1) / 10.0f;
            radius = inside(height);
            x = randFactor * std::rand() / (double)RAND_MAX + randStart + radius * std::cos(theta);
            y = randFactor * std::rand() / (double)RAND_MAX + randStart + height;
            z = randFactor * std::rand() / (double)RAND_MAX + randStart + radius * std::sin(theta);

            vertices[i++] = glm::vec3(x, y, z) * SCALE;
        }
    }

    // Outside
    for (int _height = 30, diff = 1; _height > 1; _height -= diff)
    {
        for (int j = 0; j <= RESOLUTION; j++)
        {
            theta = j * 2 * M_PI / RESOLUTION;

            height = _height / 10.0f;
            radius = f(height);
            randFactor = map(std::log(height) + 2, 0.0f, std::log(3.0f) + 2, 0.7f, 0.03f);
            randStart = -randFactor * 0.5f;

            x = randFactor * std::rand() / (double)RAND_MAX + randStart + radius * std::cos(theta);
            y = randFactor * std::rand() / (double)RAND_MAX + randStart + height;
            z = randFactor * std::rand() / (double)RAND_MAX + randStart + radius * std::sin(theta);

            vertices[i++] = glm::vec3(x, y, z) * SCALE;

            height = (_height - 1) / 10.0f;
            radius = f(height);
            x = randFactor * std::rand() / (double)RAND_MAX + randStart + radius * std::cos(theta);
            y = randFactor * std::rand() / (double)RAND_MAX + randStart + height;
            z = randFactor * std::rand() / (double)RAND_MAX + randStart + radius * std::sin(theta);

            vertices[i++] = glm::vec3(x, y, z) * SCALE;
        }
    }

    // horizontal stitch
    for (int i = 0; i < AMOUNT_OF_VERTICES; i += 2 * RESOLUTION + 2)
    {
        vertices[i + 2 * RESOLUTION] = vertices[i];
        vertices[i + 2 * RESOLUTION + 1] = vertices[i + 1];
    }

    // vertical stitch
    for (int i = 0; i < AMOUNT_OF_VERTICES - 2 * RESOLUTION - 2; i += 2)
    {
        vertices[i + 2 * RESOLUTION + 2] = vertices[i + 1];
    }

    for (int i = 0; i < AMOUNT_OF_VERTICES - 2; i++)
    {
        glm::vec3 v0 = vertices[i + (i % 2) * 2];
        glm::vec3 v1 = vertices[i + 1];
        glm::vec3 v2 = vertices[i + 2 - (i % 2) * 2];

        glm::vec3 a = v2 - v1;
        glm::vec3 b = v0 - v1;

        glm::vec3 normal = glm::cross(b, a);

        // !?
        if (glm::length(normal) < 1e-5)
            continue;

        normals[i] += normal;
        normals[i + 1] += normal;
        normals[i + 2] += normal;
    }

    // horizontal stitch
    for (int i = 0; i < AMOUNT_OF_VERTICES; i += 2 * RESOLUTION + 2)
    {
        glm::vec3 temp = normals[i];
        normals[i] += normals[i + 2 * RESOLUTION];
        normals[i + 2 * RESOLUTION] += temp;
        temp = normals[i + 1];
        normals[i + 1] += normals[i + 2 * RESOLUTION + 1];
        normals[i + 2 * RESOLUTION + 1] += temp;
    }

    // vertical stitch
    for (int i = 0; i < AMOUNT_OF_VERTICES - 2 * RESOLUTION - 2; i += 2)
    {
        glm::vec3 temp = normals[i + 1];
        normals[i + 1] += normals[i + 2 * RESOLUTION + 2];
        normals[i + 2 * RESOLUTION + 2] += temp;
    }

    for (int i = 0; i < AMOUNT_OF_VERTICES; i++)
    {
        normals[i] = glm::normalize(normals[i]);
    }

    setShaders();
    setBuffers();
}

void Volcano::setShaders()
{
    compileShaders(R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require
         #extension GL_ARB_shading_language_420pack : require
         layout(location = 0) in vec3 vpos;
         layout(location = 1) in vec3 normal;
         layout(location = 1) uniform mat4 model;
         layout(location = 2) uniform mat4 view;
         layout(location = 3) uniform mat4 projection;
         layout(location = 4) uniform mat3 normalMatrix;
         layout(location = 5) uniform vec3 cameraPos;

         out float altitude;
         out vec3 N;
         out vec3 L;
         out vec3 V;
         out vec3 fragPos;

         void main(void) {
            gl_Position = model * vec4(vpos, 1.0f);
            fragPos = gl_Position.xyz;

            N = mat3(transpose(inverse(model))) * normal;
            L = vec3(1.0f, 1.0f, 1.0f);
            V = cameraPos - gl_Position.xyz;

            gl_Position = projection * view * gl_Position;
            altitude = vpos.y;
         }

      )END",
                   R"END(

        #version 330 
        #extension GL_ARB_explicit_uniform_location : require

        layout(location = 5) uniform vec3 cameraPos;
        layout(location = 6) uniform vec3 playerPos;
        layout(location = 7) uniform float fog;
        layout(location = 8) uniform vec3 lightPositions[10];

         in float altitude;
         in vec3 N;
         in vec3 L;
         in vec3 V;
         in vec3 fragPos;
         out vec4 color;

         void main(void) {
            vec3 volColor = vec3(0.0f, 0.0f, 0.0f);

            if (altitude <= 2.0f)
                volColor = mix(volColor, vec3(0.0f, 1.0f, 0.0f), max(0.0f, altitude / 2.0f));
            else if (altitude <= 3.0f)
                volColor = mix(vec3(0.0f, 1.0f, 0.0f), vec3(0.1f), (altitude - 2.0f) / 1.0f);
            else if (altitude <= 5.0f)
                volColor = mix(vec3(0.1f), vec3(0.7f), (altitude - 3.0f) / 2.0f);
            else
                volColor = mix(vec3(0.7f), vec3(1.0f, 0.0f, 0.0f), min(1.0f, (altitude - 5.0f) / 1.0f));

            vec3 N = normalize(N);
            vec3 L = normalize(L);
            vec3 V = normalize(V);

            vec3 is = vec3(0.7f);
            vec3 id = vec3(0.5f);
            vec3 ia = vec3(0.1f);

            vec3 ks = vec3(1.0f);
            vec3 kd = volColor;
            vec3 ka = vec3(1.0f);
            float alpha = 8.0f;

            vec3 R = 2.0f * dot(L, N) * N - L;
            vec3 I = ka * ia + kd * max(dot(L, N), 0) * id + ks * pow(max(dot(R, V), 0), alpha) * is;
            
            for (int i = 0; i < 10; i++)
            {
                vec3 L = lightPositions[i] - fragPos;
                float distanceToLightSquared = dot(L, L);
                L = normalize(L);
                vec3 R = 2.0f * dot(L, N) * N - L;
                vec3 term = kd * max(dot(L, N), 0) * id + ks * pow(max(dot(R, V), 0), alpha) * is;
                term /= distanceToLightSquared + 1.0f;
                I += term;
            }

            {
                vec3 L = playerPos - fragPos;
                float distanceToLightSquared = dot(L, L);
                L = normalize(L);
                vec3 R = 2.0f * dot(L, N) * N - L;
                vec3 term = kd * max(dot(L, N), 0) * id + ks * pow(max(dot(R, V), 0), alpha) * is;
                term /= distanceToLightSquared + 1.0f;
                I += term;
            }

            vec4 thingColor = vec4(
                mod(pow((gl_FragCoord.x + 1000.0f) / 1000.f * (gl_FragCoord.y + 3000.0f) / 100.0f, 2), 1.0f),
                mod(pow((gl_FragCoord.x + 2000.0f) / 1000.f * (gl_FragCoord.y + 2000.0f) / 100.0f, 3), 1.0f),
                mod(pow((gl_FragCoord.x + 3000.0f) / 1000.f * (gl_FragCoord.y + 1000.0f) / 100.0f, 5), 1.0f),
                1.0f
                );

            color = vec4(I, 1.0f);

            if (fog > 0.5f)
            {
                vec3 distanceVec = fragPos - cameraPos;
                float distanceToCamera = length(distanceVec);
                vec4 fogColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
                color = mix(color, fogColor, min(1.0f, exp(distanceToCamera * 0.04f) - 1.0f));
            }
         }

      )END");
}

void Volcano::setBuffers()
{
    bindBuffers();
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                 // attribute 0, must match the layout in the shader.
        3,                 // size
        GL_FLOAT,          // type
        GL_FALSE,          // normalized?
        3 * sizeof(float), // stride
        (void *)0          // array buffer offset
    );

    GLuint vboNormals;
    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,                 // attribute 0, must match the layout in the shader.
        3,                 // size
        GL_FLOAT,          // type
        GL_FALSE,          // normalized?
        3 * sizeof(float), // stride
        (void *)0          // array buffer offset
    );
}

void Volcano::draw(const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &cameraPos, const glm::vec3 lightPositions[], const glm::vec3 &playerPos, bool fog)
{
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    float _fog = fog;

    bindProgram();
    bindBuffers();

    glUniformMatrix4fv(1, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(2, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(3, 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix3fv(4, 1, GL_FALSE, &normalMatrix[0][0]);
    glUniform3fv(5, 1, &cameraPos[0]);
    glUniform3fv(6, 1, &playerPos[0]);
    glUniform1fv(7, 1, &_fog);
    glUniform3fv(8, 10, &lightPositions[0][0]);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(vertices) / sizeof(glm::vec3));
}

glm::mat4 Volcano::getModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    return model;
}

glm::vec3 Volcano::vertices[];
glm::vec3 Volcano::normals[];
