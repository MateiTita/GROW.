#include "Player.h"

Player::Player(Mesh *mesh, glm::vec3 startPosition)
{
    this->mesh = mesh;
    this->position = startPosition;

    this->size = 1.0f;
    this->health = 100.0f;
    this->isDashing = false;
    this->dashTimer = 0.0f;
    this->dashCooldown = 0.0f;
}

Player::~Player()
{
}

void Player::Update(float deltaTime)
{
    if (isDashing)
    {
        dashTimer -= deltaTime;
        if (dashTimer <= 0.0f)
        {
            isDashing = false;
        }
    }

    if (dashCooldown > 0.0f)
    {
        dashCooldown -= deltaTime;
    }
}

void Player::Draw(Shader &shader)
{
    if (mesh != nullptr)
    {
        mesh->draw(shader);
    }
}

void Player::Dash()
{
    if (dashCooldown <= 0.0f)
    {
        isDashing = true;
        dashTimer = 0.2f;
        dashCooldown = 2.0f;
    }
}