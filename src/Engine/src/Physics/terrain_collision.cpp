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


namespace Engine
{
    float gravity = 0.1f;
}// namespace Engine
