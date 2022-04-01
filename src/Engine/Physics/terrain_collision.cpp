#include <Physics/terrain_collision.hpp>
#include <iostream>
#include <stdexcept>

#define mod(x) (x < 0 ? -x : x)
#define max_force_index(force)                                                                                         \
    ((mod(force.x) > mod(force.y) && mod(force.x) > mod(force.z))   ? 0                                                \
     : (mod(force.y) > mod(force.x) && mod(force.y) > mod(force.z)) ? 1                                                \
     : (mod(force.z) > mod(force.x) && mod(force.z) > mod(force.y)) ? 2                                                \
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


static Engine::ArrayIndex binary_search(const Engine::HeightMapValueArray* array, Engine::HeightMapValue& value)
{
    const auto& data = (*array).data();
    Engine::ArrayIndex left = 0, right = (*array).size() - 1, left_prev = 0;

    while (left < right && data[left] < value)
    {
        left_prev = left;
        Engine::ArrayIndex center = (left + right) / 2;
        if (data[center] < value)
            left = center + 1;
        else if (data[center] > value)
            right = center - 1;
        else
            return center;
    }

    if (data[left] <= value)
        return left;
    if (left > 0)
    {
        if (data[left - 1] <= value)
            return left - 1;
        else if (data[left_prev] <= value)
            return left_prev;
    }
    return ~0;
}

namespace Engine
{

    float gravity = 0.01;
    std::vector<ObjectParameters> check_terrain_collision(HeightMap& height_map,
                                                          const std::vector<ObjectParameters>& objects)
    {
        static Engine::ArrayIndex frame = 0;
        frame++;
        std::vector<ObjectParameters> result_object = objects;
        for (auto& object : result_object)
        {
            auto expected_position = object.position + object.force;
            HeightMapValue object_height_map_value;
            object_height_map_value.position = expected_position;
            const std::vector<HeightMapValue>* block = nullptr;
            const HeightMapValue* value = nullptr;

            // Calculating indexes of height map
            try
            {
                object_height_map_value.x = height_map.to_x_index(expected_position.x);
                object_height_map_value.y = height_map.to_y_index(expected_position.y - object.height);
                object_height_map_value.z = height_map.to_z_index(expected_position.z);

                block = &(height_map.array()[object_height_map_value.x][object_height_map_value.y]
                                            [object_height_map_value.z]);
            }
            catch (...)
            {
            }

            if (block)
            {
                Engine::ArrayIndex index = binary_search(block, object_height_map_value);
                if (index != ~0)
                    value = &(*block)[index];
            }

            if (!value)
            {
                //object.force.z -= gravity;
                if (object.force.z < 0)
                    object.force.z = 0;
            }// 10.0999	6.39999
            if (value->position.y - (object.position.y - object.height + (object.force.z - gravity)) >= 0)
            {
                object.position.y = value->position.y + object.height;
                if (object.force.z < 0)
                    object.force.z = 0;
            }
            else
            {
                object.force.z -= gravity;
            }
        }


        return result_object;
    }
}// namespace Engine
