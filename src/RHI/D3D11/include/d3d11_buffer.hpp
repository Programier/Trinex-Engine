#pragma once
#include <Graphics/rhi.hpp>
#include <d3d11.h>

namespace Engine
{
    using BufferUpdateFunction = void (*)(ID3D11Buffer* buffer, size_t, size_t, const byte*);

    class D3D11_VertexBuffer : public RHI_VertexBuffer
    {
    public:
        BufferUpdateFunction m_update_function = nullptr;
        ID3D11Buffer* m_buffer                 = nullptr;

        bool init(size_t size, const byte* data, RHIBufferType type);
        void bind(byte stream_index, size_t stride, size_t offset) override;

        void update(size_t offset, size_t size, const byte* data) override;
        ~D3D11_VertexBuffer();
    };

    class D3D11_IndexBuffer : public RHI_IndexBuffer
    {
    public:
        BufferUpdateFunction m_update_function = nullptr;
        ID3D11Buffer* m_buffer                 = nullptr;
        DXGI_FORMAT m_format                   = DXGI_FORMAT_UNKNOWN;

        bool init(size_t size, const byte* data, RHIBufferType type, IndexBufferFormat format);
        void bind(size_t offset) override;
        void update(size_t offset, size_t size, const byte* data) override;

        ~D3D11_IndexBuffer();
    };
}// namespace Engine
