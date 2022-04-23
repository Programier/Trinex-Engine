#include <Physics/terrain_collision.hpp>
#include <iostream>
#include <stdexcept>

#define mod(x) (x < 0 ? -x : x)
#define max_force_index(force)                                                                                                        \
    ((mod(force.x) > mod(force.y) && mod(force.x) > mod(force.z))   ? 0                                                               \
     : (mod(force.y) > mod(force.x) && mod(force.y) > mod(force.z)) ? 1                                                               \
     : (mod(force.z) > mod(force.x) && mod(force.z) > mod(force.y)) ? 2                                                               \
                                                                    : 0)

#define empty_force(force) (force[0] == 0 && force[1] == 0 && force[2] == 0)
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
    std::vector<ObjectParameters> check_terrain_collision(HeightMap& height_map, const std::vector<ObjectParameters>& objects)
    {
        static Engine::ArrayIndex frame = 0;
        frame++;
        std::vector<ObjectParameters> result_object = objects;
        for (auto& object : result_object)
        {
            auto expected_position = object.position + object.force;
            HeightMapValue object_height_map_value;
            object_height_map_value.position = expected_position;
            const HeightMapValue* value = nullptr;

            // Calculating indexes of height map
            try
            {
                object_height_map_value.x = height_map.to_x_index(expected_position.x);
                object_height_map_value.y = height_map.to_y_index(expected_position.y - object.height);
                object_height_map_value.z = height_map.to_z_index(expected_position.z);

                value = &(height_map.array()[object_height_map_value.x][object_height_map_value.y][object_height_map_value.z]);
            }
            catch (...)
            {
            }

            if (!value)
            {
                object.force.y -= gravity;

                continue;
            }// 10.0999	6.39999
            if (value->position.y - (object.position.y - object.height + (object.force.y - gravity)) >= 0)
            {
                object.position.y = value->position.y + object.height;
                if (object.force.y < 0)
                    object.force.y = 0;
            }
            else
            {
                object.force.y -= gravity;
            }
        }


        return result_object;
    }
}// namespace Engine
