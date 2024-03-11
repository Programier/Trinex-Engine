#pragma once
#include <Graphics/texture_2D.hpp>


namespace Engine
{
    class ENGINE_EXPORT RenderTargetTexture : public Texture2D
    {
        declare_class(RenderTargetTexture, Texture2D);

    public:
        RenderTargetTexture();
        bool is_render_target_texture() const override;
    };
}// namespace Engine
