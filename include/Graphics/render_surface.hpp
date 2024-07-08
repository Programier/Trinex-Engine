#pragma once
#include <Graphics/texture_2D.hpp>


namespace Engine
{
    class ENGINE_EXPORT RenderSurface : public Texture2D
    {
        declare_class(RenderSurface, Texture2D);

    public:
        RenderSurface();

        RenderSurface& rhi_create() override;
        RenderSurface& rhi_clear_color(const Color& color);
        RenderSurface& rhi_clear_depth_stencil(float depth, byte stencil);
    };
}// namespace Engine
