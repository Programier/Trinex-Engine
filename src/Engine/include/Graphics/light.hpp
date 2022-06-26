#pragma once
#include <Graphics/framebuffer.hpp>
#include <Graphics/shader.hpp>


namespace Engine
{
    enum class LightType : int
    {
        Point,
        Sun,
        Spot,
        Area
    };

    struct Light {

        struct Material {
            LightColor ambient;
            LightColor diffuse;
            LightColor specular;
            float shininess;

            Material& send_to_shader(const Shader& shader, const std::string& struct_name);
        };

        LightType type;
        Vector3D direction;
        float max_distance;
        Point3D position;
        LightColor ambient;
        LightColor diffuse;
        LightColor specular;
        Engine::FrameBuffer buffer;
        int pixels_block = 1;

        Light& send_to_shader(const Shader& shader, const std::string& struct_name);
    };

}// namespace Engine
