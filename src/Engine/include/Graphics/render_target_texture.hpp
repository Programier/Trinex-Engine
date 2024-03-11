#pragma once
#include <Graphics/texture_2D.hpp>


namespace Engine
{
    class ENGINE_EXPORT RenderTargetTexture : public Texture2D
    {
        declare_class(RenderTargetTexture, Texture2D);

        Vector<Texture2D*> m_sub_textures;
        class RenderTarget* m_render_target;

    public:
        RenderTargetTexture(class RenderTarget* target = nullptr);
        RenderTargetTexture& rhi_create() override;
        RenderTargetTexture& rhi_create(const byte* data, size_t size) override;
        RenderTargetTexture& update(const Size2D& size, const Offset2D& offset, const byte* data, size_t data_size,
                                    MipMapLevel mipmap = 0) override;
        bool is_render_target_texture() const override;

        size_t textures_count() const;
        Texture2D* texture_at(Index index);
    };
}// namespace Engine
