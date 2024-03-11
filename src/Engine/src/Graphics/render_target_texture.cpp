#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Graphics/render_target_texture.hpp>
#include <Graphics/rhi.hpp>


namespace Engine
{
    implement_engine_class_default_init(RenderTargetTexture);

    class RenderTargetTextureWrapper : public Texture2D
    {
    public:
        bool is_render_target_texture() const override
        {
            return true;
        }
    };

    RenderTargetTexture::RenderTargetTexture(class RenderTarget* render_target) : m_render_target(render_target)
    {
        trinex_always_check(render_target, "Render target can't be null!");

        size_t frames_count = engine_instance->rhi()->render_target_buffer_count();

        for (size_t i = 1; i < frames_count; ++i)
        {
            auto texture = Object::new_instance<EngineResource<RenderTargetTextureWrapper>>();
            texture->flags(IsSerializable, false);
            texture->flags(IsEditable, false);
            m_sub_textures.push_back(texture);
        }

        flags(IsSerializable, false);
        flags(IsEditable, false);
    }

    RenderTargetTexture& RenderTargetTexture::rhi_create()
    {
        for (Texture2D* sub_texture : m_sub_textures)
        {
            sub_texture->size           = size;
            sub_texture->base_mip_level = base_mip_level;
            sub_texture->mipmap_count   = mipmap_count;
            sub_texture->format         = format;
            sub_texture->swizzle_r      = swizzle_r;
            sub_texture->swizzle_g      = swizzle_g;
            sub_texture->swizzle_b      = swizzle_b;
            sub_texture->swizzle_a      = swizzle_a;

            sub_texture->rhi_create(nullptr, 0);
        }

        Super::rhi_create(nullptr, 0);
        return *this;
    }

    RenderTargetTexture& RenderTargetTexture::rhi_create(const byte* data, size_t size)
    {
        return rhi_create();
    }

    RenderTargetTexture& RenderTargetTexture::update(const Size2D& size, const Offset2D& offset, const byte* data,
                                                     size_t data_size, MipMapLevel mipmap)
    {
        return *this;
    }

    size_t RenderTargetTexture::textures_count() const
    {
        return 1 + m_sub_textures.size();
    }

    bool RenderTargetTexture::is_render_target_texture() const
    {
        return true;
    }

    Texture2D* RenderTargetTexture::texture_at(Index index)
    {
        if (index == 0)
        {
            return this;
        }

        --index;

        if (index >= m_sub_textures.size())
            return nullptr;

        return m_sub_textures[index];
    }
}// namespace Engine
