#include <Phisics/terrain_collision.hpp>
#include <iostream>
#include <stdexcept>

#define obj_index(coord) (height_map.to_##coord##_index(expected_position.coord))


namespace Engine
{

    float gravity = 0.01;
    std::vector<ObjectParameters> check_terrain_collision(HeightMap& height_map,
                                                          const std::vector<ObjectParameters>& objects)
    {
        return objects;
    }
}// namespace Engine
