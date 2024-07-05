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
        Swizzle swizzle_r = Swizzle::Identity;
        Swizzle swizzle_g = Swizzle::Identity;
        Swizzle swizzle_b = Swizzle::Identity;
        Swizzle swizzle_a = Swizzle::Identity;

    public:
        Texture& rhi_bind_combined(Sampler* sampler, BindLocation location);
        virtual TextureType type() const = 0;
    };

}// namespace Engine
