#include "Player.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "AGL3Window.hpp"

#include "Bubble.hpp"
#include "Camera.hpp"
#include "ClosestPointTriangle.hpp"

static int initMesh()
{
    int i = 0;
    const int RES = Player::RESOLUTION;
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

            Player::vertices[i++] = glm::vec3(x, y, z);

            radius = std::sin(vTheta2);

            x = std::cos(hTheta) * radius;
            y = std::cos(vTheta2);
            z = std::sin(hTheta) * radius;

            Player::vertices[i++] = glm::vec3(x, y, z);
        }
    }

    return 0;
}

Player::Player(glm::vec3 position, float radius) : position(position)
{
    static int _init = initMesh();

    desiredPosition = glm::vec3(0.0f);
    yaw = 180.0f;
    pitch = 0.0f;
    speed = 3.0f;
    camera = nullptr;
    yawSpeed = 50.0f;
    tpCamera = true;
    dead = false;
    points = 0;
    level = 1;

    setShaders();
    setBuffers();

    up = glm::vec3(0.0f, 1.0f, 0.0f);
}

void Player::setShaders()
{
    compileShaders(R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require
         #extension GL_ARB_shading_language_420pack : require
         layout(location = 0) in vec3 vpos;
         layout(location = 1) uniform mat4 model;
         layout(location = 2) uniform mat4 view;
         layout(location = 3) uniform mat4 projection;
         layout(location = 5) uniform vec3 cameraPos;

         out vec3 N;
         out vec3 L;
         out vec3 V;

         void main(void) {
            gl_Position = model * vec4(vpos, 1.0f);

            N = mat3(transpose(inverse(model))) * normalize(vpos);
            L = vec3(1.0f, 1.0f, 1.0f);
            V = cameraPos - gl_Position.xyz;

            gl_Position = projection * view * gl_Position;
         }

      )END",
                   R"END(

         #version 330 
         #extension GL_ARB_explicit_uniform_location : require

         in vec3 N;
         in vec3 L;
         in vec3 V;
         out vec4 color;

         void main(void) {
            vec3 N = normalize(N);
            vec3 L = normalize(L);
            vec3 V = normalize(V);
            vec3 R = 2.0f * dot(L, N) * N - L;

            vec3 is = vec3(1.0f, 1.0f, 1.0f);
            vec3 id = vec3(1.0f, 1.0f, 1.0f);
            vec3 ia = vec3(0.2f);

            vec3 ks = vec3(1.0f);
            vec3 kd = vec3(1.0f);
            vec3 ka = vec3(1.0f);
            float alpha = 32.0f;

            vec3 I = ka * ia + kd * max(dot(L, N), 0) * id + ks * pow(max(dot(R, V), 0), alpha) * is;

            color = vec4(I, 1.0f);
         } 

      )END");
}

void Player::setBuffers()
{
    bindBuffers();

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,        // attribute 0, must match the layout in the shader.
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );
}

glm::mat4 Player::getModelMatrix(int sphere_index, bool bigger = false) const
{
    glm::mat4 model = glm::mat4(1.0f);

    if (sphere_index == 0)
    {
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(0.1f + bigger));
    }
    else
    {
        float relativeDistance = 0.1f;
        glm::vec3 relativePosition = glm::vec3(
            -std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch)),
            std::sin(glm::radians(pitch)),
            -std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch))
        ) * relativeDistance;

        model = glm::translate(model, position + relativePosition);
        model = glm::scale(model, glm::vec3(0.04f));
    }
    
    model = glm::rotate(model, glm::radians(-yaw), glm::vec3(0.0f, 1.0f, 0.0f));

    return model;
}

void Player::draw(const glm::mat4 &view, const glm::mat4 &projection, bool highlight)
{
    glm::mat4 model = getModelMatrix(0, highlight);

    bindProgram();
    bindBuffers();

    glUniformMatrix4fv(1, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(2, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(3, 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(5, 1, &camera->position[0]);

    glLineWidth(1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(vertices));

    model = getModelMatrix(1, highlight);

    glUniformMatrix4fv(1, 1, GL_FALSE, &model[0][0]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(vertices));
}

void Player::update(float deltaTime, const glm::vec4 bubblePositions[], const Volcano &volcano)
{
    float desiredPositionLength = glm::length(desiredPosition);
    if (desiredPositionLength > 10e-6)
    {
        float velocity = speed * deltaTime;
        if (desiredPositionLength > 1.0f)
            desiredPosition = glm::normalize(desiredPosition);
        pitch = glm::dot(front, desiredPosition) * 90.0f;
        glm::vec3 deltaPos = desiredPosition * velocity;
        
        for (int i = 0; i < Bubble::AMOUNT; i++)
        {
            glm::vec3 distanceVec = glm::vec3(bubblePositions[i]) - position + deltaPos;
            float distanceSq = dot(distanceVec, distanceVec);
            if (distanceSq < 1.0f && bubbleCollision(position + deltaPos, bubblePositions[i]))
            {
                if (Bubble::properties[i][Bubble::Property::SPECIAL] > 0.5f)
                {
                    Bubble::initInstance(i);
                    points += level;
                }
                else
                {
                    dead = true;
                    return;
                }
            }
        }

        if (volcanoCollision(position + deltaPos, volcano))
        {
            desiredPosition = glm::vec3(0.0f);
            updateCameraPosition();
            return;
        }

        position += deltaPos;
        desiredPosition -= deltaPos;
    }

    float yawDiff = desiredYaw - yaw;
    if (std::abs(yawDiff) > std::abs(yawDiff - 360.0f))
    {
        yawDiff -= 360.0f;
    }
    else if (std::abs(yawDiff) > std::abs(yawDiff + 360.0f))
    {
        yawDiff += 360.0f;
    }

    float yawSign = yawDiff > 0.0f ? 1.0f : -1.0f;
    yawDiff = std::abs(yawDiff);
    yaw += yawSign * std::min(yawDiff, yawSpeed) * deltaTime;

    yaw += 360.0f * (yaw < -180.0f);
    yaw -= 360.0f * (yaw >  180.0f);

    front = glm::normalize(glm::vec3(
        cos(glm::radians(yaw)),
        0,
        sin(glm::radians(yaw))));

    right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));

    updateCameraPosition();
}

void Player::processKeyboard(int key, float deltaTime)
{
    glm::vec3 deltaPos = glm::vec3(0.0f);
    float acceleration = 0.01f * speed;

    if (key == Key::W)
        deltaPos += front * acceleration;
    if (key == Key::S)
        deltaPos -= front * acceleration;
    if (key == Key::A)
        deltaPos -= right * acceleration;
    if (key == Key::D)
        deltaPos += right * acceleration;
    if (key == Key::SPACE)
        deltaPos += up * acceleration;
    if (key == Key::LEFT_CONTROL)
        deltaPos -= up * acceleration;

    if (key == Key::TAB)
        tpCamera = !tpCamera;

    desiredPosition += deltaPos;
}

void Player::updateCameraPosition()
{
    if (tpCamera)
    {
        float cameraDistanceToPlayer = 0.5f;
        glm::vec3 relativeCameraPosition = glm::vec3(
            std::cos(glm::radians(desiredYaw)) * -std::cos(glm::radians(camera->pitch)),
            -std::sin(glm::radians(camera->pitch)),
            std::sin(glm::radians(desiredYaw)) * -std::cos(glm::radians(camera->pitch))
        ) * cameraDistanceToPlayer;
        cameraPosition = position + relativeCameraPosition;
    }
    else
    {
        cameraPosition = position;
    }
    camera->position = cameraPosition;
}

bool Player::bubbleCollision(const glm::vec3 &newPos, const glm::vec4 &bubblePos) const
{
    glm::vec3 distanceVec = glm::vec3(bubblePos) - newPos;
    float radiusSum = bubblePos.w + 0.1f;
    if (glm::dot(distanceVec, distanceVec) < radiusSum * radiusSum)
    {
        return true;
    }

    glm::mat4 miniModel = getModelMatrix(1);
    glm::vec3 miniPos = miniModel * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    miniPos += newPos - position;
    distanceVec = glm::vec3(bubblePos) - miniPos;
    radiusSum = bubblePos.w + 0.04f;
    if (glm::dot(distanceVec, distanceVec) < radiusSum * radiusSum)
    {
        return true;
    }

    return false;
}

bool Player::volcanoCollision(const glm::vec3 &newPos, const Volcano &volcano) const
{
    for (int i = 0; i < sizeof(volcano.vertices) / sizeof(glm::vec3) - 2; i++)
    {
        glm::vec3 closestPoint = closestPointTriangle(newPos, volcano.vertices[i], volcano.vertices[i + 1], volcano.vertices[i + 2]);
        glm::vec3 distanceVec = closestPoint - newPos;
        float distanceSq = glm::dot(distanceVec, distanceVec);
        if (distanceSq < 0.01f)
        {
            return true;
        }

        glm::mat4 miniModel = getModelMatrix(1);
        glm::vec3 miniPos = miniModel * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        miniPos += newPos - position;
        distanceVec = closestPoint - miniPos;
        distanceSq = glm::dot(distanceVec, distanceVec);
        if (distanceSq < 0.04f * 0.04f)
        {
            return true;
        }
    }

    return false;
}

glm::vec3 Player::vertices[];