#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class PlayerMovement
{
public:
    glm::vec3 Position;
    glm::vec3 Velocity;

    float DashSpeed = 25.0f;
    float DashDuration = 0.2f;
    float DashCooldown = 1.5f;

    bool IsDashing = false;
    float DashTimeLeft = 0.0f;
    float CooldownTimer = 0.0f;
    glm::vec3 DashDirection;

    float MoveSpeed = 5.0f;
    float WaterDrag = 3.0f;

    PlayerMovement(glm::vec3 startPos) : Position(startPos), Velocity(0.0f) {}

    glm::quat Rotation;
    float TurnSpeed = 10.0f;

    PlayerMovement(glm::vec3 startPos) : Position(startPos), Velocity(0.0f)
    {
        Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }

    struct InputState
    {
        bool W, A, S, D;
        bool Rise;
        bool Sink;
        bool Dash;
    } Input;

    void Update(float deltaTime, glm::vec3 camFront, glm::vec3 camRight, glm::vec3 camUp);

    void TryStartDash(glm::vec3 camFront);

    glm::mat4 GetModelMatrix()
    {
        glm::mat4 model = glm::mat4(1.0f);
        return glm::translate(model, Position);

        model = model * glm::toMat4(Rotation);
        return model;
    }
};