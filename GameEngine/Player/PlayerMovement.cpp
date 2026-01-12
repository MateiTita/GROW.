#include "PlayerMovement.h"

void PlayerMovement::TryStartDash(glm::vec3 camFront)
{
    if (CooldownTimer <= 0.0f && !IsDashing)
    {
        IsDashing = true;
        DashTimeLeft = DashDuration;
        CooldownTimer = DashCooldown;

        DashDirection = glm::normalize(camFront);
    }
}

void PlayerMovement::Update(float deltaTime, glm::vec3 camFront, glm::vec3 camRight, glm::vec3 camUp)
{

    if (CooldownTimer > 0.0f)
    {
        CooldownTimer -= deltaTime;
    }

    if (IsDashing)
    {
        Velocity = DashDirection * DashSpeed;

        DashTimeLeft -= deltaTime;
        if (DashTimeLeft <= 0.0f)
        {
            IsDashing = false;
            Velocity *= 0.5f;
        }
    }
    else
    {
        glm::vec3 moveDir = glm::vec3(0.0f);

        if (Input.W)
            moveDir += camFront;
        if (Input.S)
            moveDir -= camFront;
        if (Input.D)
            moveDir += camRight;
        if (Input.A)
            moveDir -= camRight;
        if (Input.Rise)
            moveDir += camUp;
        if (Input.Sink)
            moveDir -= camUp;

        if (glm::length(moveDir) > 0.0f)
        {
            moveDir = glm::normalize(moveDir);
        }

        glm::vec3 targetVelocity = moveDir * MoveSpeed;
        float interpolationFactor = deltaTime * WaterDrag;
        if (interpolationFactor > 1.0f)
            interpolationFactor = 1.0f;

        Velocity = glm::mix(Velocity, targetVelocity, interpolationFactor);

        if (glm::length(Velocity) > 0.1f)
        {
            glm::vec3 direction = glm::normalize(Velocity);
            glm::mat4 rotationMatrix = glm::lookAt(glm::vec3(0.0f), direction, glm::vec3(0, 1, 0));
            rotationMatrix = glm::inverse(rotationMatrix);
            glm::quat targetRotation = glm::quat_cast(rotationMatrix);
            Rotation = glm::slerp(Rotation, targetRotation, deltaTime * TurnSpeed);
        }

        Position += Velocity * deltaTime;
    }
}