#include <Core/constants.hpp>
#include <thread>

namespace Engine
{
    const unsigned int Constants::processor_count = std::thread::hardware_concurrency();
    const glm::vec3 Constants::OX(1.f, 0.f, 0.f);
    const glm::vec3 Constants::OY(0.f, 1.f, 0.f);
    const glm::vec3 Constants::OZ(0.f, 0.f, 1.f);
    const float Constants::PI = glm::pi<float>();
    const float Constants::E = glm::e<float>();
    const glm::mat4 Constants::identity_matrix = glm::mat4(1.0f);
    const glm::mat4 Constants::zero_matrix = glm::mat4();
    const glm::vec4 Constants::identity_vector = glm::vec4(1.f, 1.f, 1.f, 1.f);
    const glm::vec4 Constants::zero_vector = glm::vec4(0.f, 0.f, 0.f, 0.f);
    const float Constants::min_positive_float = 0.000001f;
    const Vector3D Constants::min_positive_vector = Vector3D(0.000001f);
}// namespace Engine
