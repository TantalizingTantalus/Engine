// Transform.h
#pragma once  // or use include guards

#include <glm/glm.hpp>
#include <vector>

struct Transform {
    glm::vec3 pos;
    glm::vec3 eulerRot;
    glm::vec3 scale;
    glm::mat4 modelMatrix;
    Transform() : pos(0.0f), eulerRot(0.0f), scale(1.0f), modelMatrix(1.0f) {}
};