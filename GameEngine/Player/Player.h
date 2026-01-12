#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Model Loading/mesh.h"
#include "Shaders/shader.h"

class Player
{
public:
    Player(Mesh *mesh, glm::vec3 startPosition);
    ~Player();

    void Update();
    void Draw(Shader shader);

    glm::vec3 position;
    float size;
    float health;

private:
    Mesh *mesh;
};