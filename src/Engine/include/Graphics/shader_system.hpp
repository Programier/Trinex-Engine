#pragma once

#include <Graphics/shader.hpp>


namespace Engine
{
    namespace ShaderSystem
    {
        namespace SkyBox
        {
            extern Shader shader;
            extern const std::string projview;
        }// namespace SkyBox

        namespace Text
        {
            extern Shader shader;
            extern const std::string projview;
            extern const std::string color;
        }// namespace Text

        namespace Scene
        {
            extern Shader shader;
            extern const std::string projview;
            extern const std::string model;
            extern const std::string light_projview;
            extern const std::string camera_pos;
            extern const std::string lighting;
            extern const std::string transposed_inversed_model;
        }// namespace Scene

        namespace Line
        {
            extern Shader shader;
            extern const std::string projview;
            extern const std::string model;
            extern const std::string color;
        }// namespace Line

        namespace Depth
        {
            extern Shader shader;
            extern const std::string projview;
            extern const std::string model;
        }// namespace Depth

        namespace DepthRenderer
        {
            extern Shader shader;
            extern const std::string power;
        }// namespace Depth

        void init();
    }// namespace ShaderSystem
}// namespace Engine
