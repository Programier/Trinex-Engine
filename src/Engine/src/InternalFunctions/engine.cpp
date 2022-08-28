#define ENGINE_API

#include <engine.hpp>
#include <glm/gtc/quaternion.hpp>
#include <opengl.hpp>
#include <thread>
#include <unordered_map>

namespace Engine
{

    const std::size_t COLOR_BUFFER_BIT = 1;
    const std::size_t DEPTH_BUFFER_BIT = 2;
    const unsigned int processor_count = std::thread::hardware_concurrency();
    const glm::vec3 OX(1.f, 0.f, 0.f);
    const glm::vec3 OY(0.f, 1.f, 0.f);
    const glm::vec3 OZ(0.f, 0.f, 1.f);
    const float PI = glm::pi<float>();
    const float E = glm::e<float>();
    const glm::mat4 identity_matrix = glm::mat4(1.0f);
    const glm::mat4 zero_matrix = glm::mat4();
    const glm::vec4 identity_vector = glm::vec4(1.f, 1.f, 1.f, 1.f);
    const glm::vec4 zero_vector = glm::vec4(0.f, 0.f, 0.f, 0.f);
    const std::string ShaderModeValue = "RenderType";

    static Shader shader;

    void init_shader()
    {
        {
            shader.load("Shaders/main.vert", "Shaders/main.frag");
            for (int i = 0; i < 3; i++) shader.set("texture" + std::to_string(i), i);
        }
    }

    const Shader& engine_shader()
    {
        return shader;
    }

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
        return glm::dot(first, second);
    }

    float angle_between(glm::vec3 first, glm::vec3 second)
    {
        return glm::acos(scalar_mult(glm::normalize(first), glm::normalize(second)));
    }

    glm::vec3 remove_coord(const glm::vec3& vector, const Coord& coord)
    {
        auto result = vector;
        result[static_cast<int>(coord)] = 0.f;
        return glm::normalize(result);
    }

    bool get_bit(const std::size_t& value, int bit)
    {
        return (value >> (bit - 1)) & 1;
    }

    namespace OpenGL
    {
        int compare_func(const CompareFunc& func)
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
