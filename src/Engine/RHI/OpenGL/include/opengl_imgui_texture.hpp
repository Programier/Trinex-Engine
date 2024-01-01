#pragma once
#include <Graphics/rhi.hpp>

namespace Engine
{
    struct OpenGL_ImGuiTextureInterface : public RHI_ImGuiTexture {
        virtual uint_t texture_id() = 0;
        virtual uint_t sampler_id() = 0;


        FORCE_INLINE void* handle() override
        {
            return this;
        }

        FORCE_INLINE void destroy_now() override
        {}

        virtual ~OpenGL_ImGuiTextureInterface()
        {}
    };

    struct OpenGL_ImGuiTextureBasic : public OpenGL_ImGuiTextureInterface {
        uint_t _M_texture_id = 0;
        uint_t _M_sampler_id = 0;

        FORCE_INLINE OpenGL_ImGuiTextureBasic(uint_t texture = 0, uint_t sampler = 0)
            : _M_texture_id(texture), _M_sampler_id(sampler)
        {}

        FORCE_INLINE uint_t texture_id() override
        {
            return _M_texture_id;
        }

        FORCE_INLINE uint_t sampler_id() override
        {
            return _M_sampler_id;
        }
    };


    struct OpenGL_ImGuiTexture : public OpenGL_ImGuiTextureInterface {
        class Texture* _M_texture = nullptr;
        class Sampler* _M_sampler = nullptr;


        FORCE_INLINE OpenGL_ImGuiTexture(class Texture* texture, class Sampler* sampler)
            : _M_texture(texture), _M_sampler(sampler)
        {}

        uint_t texture_id() override;
        uint_t sampler_id() override;
    };
}// namespace Engine
