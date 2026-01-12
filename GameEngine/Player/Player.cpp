#include "Player.h"

Player::Player(Mesh *mesh, glm::vec3 startPosition)
{
    this->mesh = mesh;
    this->position = startPosition;

    this->size = 1.0f;
    this->health = 100.0f;
}

Player::~Player()
{
}

void Player::Update()
{
}

void Player::Draw(Shader shader)
{
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, position);

    model = glm::scale(model, glm::vec3(size));

    glUniformMatrix4fv(glGetUniformLocation(shader.getId(), "model"), 1, GL_FALSE, &model[0][0]);

    if (mesh != nullptr)
    {
        mesh->draw(shader);
    }
}