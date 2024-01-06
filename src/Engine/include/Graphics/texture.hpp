#pragma once
#include <Core/color_format.hpp>
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
        ColorFormat format         = ColorFormat::R8G8B8A8Unorm;
        Swizzle swizzle_r          = Swizzle::Identity;
        Swizzle swizzle_g          = Swizzle::Identity;
        Swizzle swizzle_b          = Swizzle::Identity;
        Swizzle swizzle_a          = Swizzle::Identity;

    protected:
        bool _M_use_for_render_target = false;

    public:
        Texture();
        delete_copy_constructors(Texture);
        const Texture& bind_combined(Sampler* sampler, BindLocation location) const;
        Texture& generate_mipmap();
        Texture& setup_render_target_texture();
        bool is_render_target_texture() const;


        Size2D mip_size(MipMapLevel level = 0) const;
        bool archive_process(Archive* archive) override;
        virtual TextureType type() const = 0;
        ~Texture();
    };

}// namespace Engine
