#pragma once

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "Model Loading\mesh.h"
#include "Shaders\shader.h"

class Player
{
public:
    Player(Mesh *mesh, glm::vec3 startPosition);
    ~Player();

    void Update(float deltaTime);
    void Draw(Shader &shader);
    void Dash();

    glm::vec3 position;
    float size;
    float health;
    bool isDashing;
    float dashTimer;
    float dashCooldown;

private:
    Mesh *mesh;
};