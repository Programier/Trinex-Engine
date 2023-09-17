#pragma once
#include <Core/texture_types.hpp>
#include <functional>
#include <opengl_object.hpp>
#include <opengl_api.hpp>
#include <opengl_color_format.hpp>

namespace Engine
{
    struct TextureSize {
        GLsizei width;
        GLsizei height;
    };

    struct OpenGL_Texture : public OpenGL_Object {
        GLsizei _M_width, _M_height;
        GLenum _M_texture_type;

        OpenGL_ColorFormat _M_format;

        byte _M_use_sampler_mode_linear : 1 = 0;

        implement_opengl_instance_hpp();
        inline TextureSize size() const
        {
            return {_M_width, _M_height};
        }

        template<typename ReturnType, typename... Args>
        typename std::enable_if<std::is_same_v<ReturnType, void>, OpenGL_Texture&>::type
        apply_function(ReturnType (*function)(Args...), Args... args)
        {
            API->internal_bind_texture(this);
            function(args...);
            API->internal_bind_texture(nullptr);
            return *this;
        }

        template<typename ReturnType, typename... Args>
        typename std::enable_if<!std::is_same_v<ReturnType, void>, ReturnType>::type
        apply_function(ReturnType (*function)(Args...), Args... args)
        {
            API->internal_bind_texture(this);
            auto result = function(args...);
            API->internal_bind_texture(nullptr);
            return result;
        }

        ~OpenGL_Texture();
    };
}// namespace Engine
