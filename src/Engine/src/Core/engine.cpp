#include <Core/engine.hpp>
#include <glm/gtc/quaternion.hpp>
#include <opengl.hpp>
#include <thread>
#include <unordered_map>

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


    const std::string ShaderModeValue = "RenderType";

    static Shader shader;

    ENGINE_EXPORT const Shader& engine_shader()
    {
        return shader;
    }

    ENGINE_EXPORT glm::vec3 get_rotation_from_matrix(const glm::mat4& m)
    {
        return glm::eulerAngles(glm::quat(m));
    }

    ENGINE_EXPORT glm::mat4 quaternion_matrix(const glm::vec3& rotation)
    {
        return glm::mat4_cast(glm::quat(rotation));
    }

    ENGINE_EXPORT float scalar_mult(const glm::vec3& first, const glm::vec3& second)
    {
        return glm::dot(first, second);
    }

    ENGINE_EXPORT float angle_between(glm::vec3 first, glm::vec3 second)
    {
        return glm::acos(scalar_mult(glm::normalize(first), glm::normalize(second)));
    }

    ENGINE_EXPORT glm::vec3 remove_coord(const glm::vec3& vector, const Coord& coord)
    {
        auto result = vector;
        result[static_cast<int>(coord)] = 0.f;
        return glm::normalize(result);
    }

    ENGINE_EXPORT bool get_bit(const std::size_t& value, int bit)
    {
        return (value >> (bit - 1)) & 1;
    }

    namespace OpenGL
    {
        ENGINE_EXPORT int compare_func(const CompareFunc& func)
        {
            static const std::unordered_map<CompareFunc, int> OpenGL_compare_func = {
                    {CompareFunc::Lequal, GL_LEQUAL}, {CompareFunc::Gequal, GL_GEQUAL},
                    {CompareFunc::Less, GL_LESS},     {CompareFunc::Greater, GL_GREATER},
                    {CompareFunc::Equal, GL_EQUAL},   {CompareFunc::NotEqual, GL_NOTEQUAL},
                    {CompareFunc::Always, GL_ALWAYS}, {CompareFunc::Never, GL_NEVER}};
            return OpenGL_compare_func.at(func);
        }

    }// namespace OpenGL
}// namespace Engine
