#pragma once
#include <Graphics/rhi.hpp>
#include <d3d11.h>

namespace Engine
{
    class D3D11_Sampler : public RHI_Sampler
    {
    public:
        ID3D11SamplerState* m_sampler = nullptr;

        bool init(const Sampler* sampler);
        void bind(BindLocation location) override;
        ~D3D11_Sampler();
    };
}// namespace Engine
