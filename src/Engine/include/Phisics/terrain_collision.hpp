#pragma once
#include <Graphics/heightmap.hpp>
#include <glm/glm.hpp>
#include <vector>


namespace Engine
{
    typedef struct {
        glm::vec3 position = {0, 0, 0};
        glm::vec3 force = {0, 0, 0};
        float height = 0;
    } ObjectParameters;

    extern float gravity;

    std::vector<ObjectParameters> check_terrain_collision(HeightMap& height_map,
                                                          const std::vector<ObjectParameters>& objects);
}// namespace Engine
