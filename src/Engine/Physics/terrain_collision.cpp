#include <Physics/terrain_collision.hpp>
#include <iostream>
#include <stdexcept>

#define mod(x) (x < 0 ? -x : x)
#define max_force_index(force)                                                                                         \
    ((mod(force.x) > mod(force.y) && mod(force.x) > mod(force.z))   ? 0                                                \
     : (mod(force.y) > mod(force.x) && mod(force.y) > mod(force.z)) ? 1                                                \
     : (mod(force.z) > mod(force.x) && mod(force.z) > mod(force.y)) ? 2                                                \
                                                                    : 0)


#define print_force(force) std::clog << force.x << "\t " << force.y << "\t " << force.z << std::endl

static void normalize_force(Engine::Force& force)
{
    std::size_t force_max_index = max_force_index(force);
    if (force[force_max_index] == 0)
        return;
    force /= force[force_max_index];
}

namespace Engine
{

    float gravity = 0.01;
    std::vector<ObjectParameters> check_terrain_collision(HeightMap& height_map,
                                                          const std::vector<ObjectParameters>& objects)
    {
        static std::size_t frame = 1;
        std::vector<ObjectParameters> result_object = objects;
        for (auto& object : result_object)
        {
            auto expected_position = object.position + object.force;
            HeightMapValue force(0.0f, 0.0f, 0.0f);

            // Calculating indexes of height map
            try
            {
                std::size_t x_index = height_map.to_x_index(expected_position.x);
                std::size_t y_index = height_map.to_y_index(expected_position.y - object.height);
                std::size_t z_index = height_map.to_z_index(expected_position.z);
                force = height_map.array()[x_index][y_index][z_index];
            }
            catch (...)
            {
                // Pass
                force = {0.f, 1.f, 0.f};
                force *= (object.position.y < 0);
            }

            // Normalizing force vertor
            normalize_force(force);
            if (glm::abs(force.y) != 0 && object.force.z < 0)
            {
                object.position.y -= object.force.z;
                object.force.z = 0;
            }
            else
                object.force.z -= gravity;
        }

        return result_object;
    }
}// namespace Engine
