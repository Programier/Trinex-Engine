#pragma once
#include <Graphics/rhi.hpp>
#include <d3d11.h>

namespace Engine
{
    class D3D11_Texture : public RHI_Texture
    {
    public:
        void clear_color(const Color& color) override;
        void clear_depth_stencil(float depth, byte stencil) override;
    };

    class D3D11_Texture2D : public D3D11_Texture
    {
    public:
        ID3D11Texture2D* m_texture       = nullptr;
        ID3D11ShaderResourceView* m_view = nullptr;

        bool init(const Texture2D* texture);
        void bind(BindLocation location) override;
        void bind_combined(RHI_Sampler* sampler, BindLocation location) override;
        ~D3D11_Texture2D();
    };
}// namespace Engine