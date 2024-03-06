#pragma once
#include <Graphics/rhi.hpp>

namespace Engine
{
    struct OpenGL_ImGuiTextureInterface : public RHI_ImGuiTexture {
        virtual uint_t texture_id() = 0;

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
        uint_t m_texture_id = 0;

        FORCE_INLINE OpenGL_ImGuiTextureBasic(uint_t texture = 0) : m_texture_id(texture)
        {}

        FORCE_INLINE uint_t texture_id() override
        {
            return m_texture_id;
        }
    };

    struct OpenGL_ImGuiTexture : public OpenGL_ImGuiTextureInterface {
        class Texture* m_texture = nullptr;

        FORCE_INLINE OpenGL_ImGuiTexture(class Texture* texture) : m_texture(texture)
        {}

        uint_t texture_id() override;
    };
}// namespace Engine
