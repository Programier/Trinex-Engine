#pragma once
#include <Core/render_resource.hpp>

struct ImGuiContext;

namespace Engine
{
    class Sampler;

    class ENGINE_EXPORT Texture : public BindedRenderResource
    {
        declare_class(Texture, BindedRenderResource);

    public:
        Size2D size                = {1, 1};
        MipMapLevel base_mip_level = 0;
        MipMapLevel mipmap_count   = 1;
        ColorFormat format         = ColorFormat::R8G8B8A8;
        Swizzle swizzle_r          = Swizzle::Identity;
        Swizzle swizzle_g          = Swizzle::Identity;
        Swizzle swizzle_b          = Swizzle::Identity;
        Swizzle swizzle_a          = Swizzle::Identity;

    public:
        Texture& generate_mipmap();
        Texture& rhi_bind_combined(Sampler* sampler, BindLocation location);
        virtual bool is_render_target_texture() const;

        Size2D mip_size(MipMapLevel level = 0) const;
        virtual TextureType type() const = 0;
    };

}// namespace Engine
