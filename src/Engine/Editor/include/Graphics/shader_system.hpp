#pragma once

#include <Graphics/shader.hpp>


namespace Engine
{
    namespace ShaderSystem
    {
        namespace SkyBox
        {
            extern ENGINE_EXPORT Shader shader;
            extern ENGINE_EXPORT const std::string projview;
        }// namespace SkyBox

        namespace Text
        {
            extern ENGINE_EXPORT Shader shader;
            extern ENGINE_EXPORT const std::string projview;
            extern ENGINE_EXPORT const std::string color;
        }// namespace Text

        namespace Scene
        {
            extern ENGINE_EXPORT Shader shader;
            extern ENGINE_EXPORT const std::string projview;
            extern ENGINE_EXPORT const std::string model;
            extern ENGINE_EXPORT const std::string light_projview;
            extern ENGINE_EXPORT const std::string camera_pos;
            extern ENGINE_EXPORT const std::string lighting;
            extern ENGINE_EXPORT const std::string transposed_inversed_model;
        }// namespace Scene

        namespace Line
        {
            extern ENGINE_EXPORT Shader shader;
            extern ENGINE_EXPORT const std::string projview;
            extern ENGINE_EXPORT const std::string model;
            extern ENGINE_EXPORT const std::string color;
        }// namespace Line

        namespace Depth
        {
            extern ENGINE_EXPORT Shader shader;
            extern ENGINE_EXPORT const std::string projview;
            extern ENGINE_EXPORT const std::string model;
        }// namespace Depth

        namespace DepthRenderer
        {
            extern ENGINE_EXPORT Shader shader;
            extern ENGINE_EXPORT const std::string power;
        }// namespace Depth

        ENGINE_EXPORT void init();
    }// namespace ShaderSystem
}// namespace Engine
