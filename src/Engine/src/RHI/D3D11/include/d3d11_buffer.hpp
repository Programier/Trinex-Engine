#pragma once
#include <Graphics/rhi.hpp>
#include <d3d11.h>

namespace Engine
{
    class D3D11_VertexBuffer : public RHI_VertexBuffer
    {
    public:
        void (*update_function)(ID3D11Buffer* buffer, size_t, size_t, const byte*) = nullptr;
        ID3D11Buffer* m_buffer                                                     = nullptr;

        bool init(size_t size, const byte* data, RHIBufferType type);
        void bind(byte stream_index, size_t stride, size_t offset) override;

        void update(size_t offset, size_t size, const byte* data) override;
        ~D3D11_VertexBuffer();
    };
}// namespace Engine
