#include <Graphics/light.hpp>

namespace Engine
{
    Light& Light::send_to_shader(const Shader& shader, const std::string& struct_name)
    {
        shader.set(struct_name + ".position", position)
                .set(struct_name + ".ambient", ambient)
                .set(struct_name + ".diffuse", diffuse)
                .set(struct_name + ".specular", specular)
                .set(struct_name + ".pixels_block", pixels_block)
                .set(struct_name + ".type", static_cast<int>(type))
                .set(struct_name + ".direction", direction)
                .set(struct_name + ".max_distance", max_distance);
        return *this;
    }

    Light::Material& Light::Material::send_to_shader(const Shader& shader, const std::string& struct_name)
    {
        shader.set(struct_name + ".ambient", ambient)
                .set(struct_name + ".diffuse", diffuse)
                .set(struct_name + ".specular", specular)
                .set(struct_name + ".shininess", shininess);
        return *this;
    }
}// namespace Engine
