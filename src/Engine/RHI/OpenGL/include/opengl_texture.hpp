#pragma once
#include <Graphics/rhi.hpp>
#include <opengl_color_format.hpp>
#include <opengl_headers.hpp>


namespace Engine
{
    struct OpenGL_Texture : public RHI_Texture {
        OpenGL_ColorInfo _M_format;
        GLuint _M_type = 0;
        GLuint _M_id   = 0;
        Size2D _M_size;

        void bind(BindLocation location) override;
        void generate_mipmap() override;
        void bind_combined(RHI_Sampler* sampler, BindLocation location) override;
        void update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap,
                               const byte* data) override;

        void init(const Texture* texture, const byte* data);

        ~OpenGL_Texture();
    };
}// namespace Engine
