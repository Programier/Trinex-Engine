#include <BasicFunctional/basic_functional.hpp>
#include <glm/gtc/quaternion.hpp>
#include <thread>

namespace Engine
{
    const unsigned int processor_count = std::thread::hardware_concurrency();

    glm::vec3 get_rotation_from_matrix(const glm::mat4& m)
    {
        return glm::eulerAngles(glm::quat(m));
    }

    glm::mat4 quaternion_matrix(const glm::vec3& rotation)
    {
        return glm::mat4_cast(glm::quat(rotation));
    }

    float scalar_mult(const glm::vec3& first, const glm::vec3& second)
    {
        return first[0] * second[0] + first[1] * second[1] + first[2] * second[2];
    }

    float angle_between(glm::vec3 first, glm::vec3 second)
    {
        return glm::acos(scalar_mult(glm::normalize(first), glm::normalize(second)));
    }
}// namespace Engine
