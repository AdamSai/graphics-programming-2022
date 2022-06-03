#ifndef ITU_GRAPHICS_PROGRAMMING_VERTEX_H
#define ITU_GRAPHICS_PROGRAMMING_VERTEX_H

#include <vector>
#include <iostream>
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

struct Vertex
{
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
};

#endif //ITU_GRAPHICS_PROGRAMMING_VERTEX_H
