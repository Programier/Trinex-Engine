#include <d3d11_api.hpp>
#include <d3d11_buffer.hpp>

namespace Engine
{
    void static static_update(ID3D11Buffer* buffer, size_t offset, size_t size, const byte* data)
    {
        D3D11_BOX update_box = {};
        update_box.left      = offset;
        update_box.right     = offset + size;
        update_box.top       = 0;
        update_box.bottom    = 1;
        update_box.front     = 0;
        update_box.back      = 1;
        DXAPI->m_context->UpdateSubresource(buffer, 0, &update_box, data, 0, 0);
    }

    void static dynamic_update(ID3D11Buffer* buffer, size_t offset, size_t size, const byte* data)
    {
        D3D11_MAPPED_SUBRESOURCE mapped_resource{};

        HRESULT hr = DXAPI->m_context->Map(buffer, 0, D3D11_MAP_WRITE, 0, &mapped_resource);
        if (SUCCEEDED(hr))
        {
            memcpy(mapped_resource.pData, data, size);
            DXAPI->m_context->Unmap(buffer, 0);
        }
    }

    bool D3D11_VertexBuffer::init(size_t size, const byte* data, RHIBufferType type)
    {
        D3D11_BUFFER_DESC desc = {};
        if (type == RHIBufferType::Dynamic)
        {
            desc.Usage          = D3D11_USAGE_DYNAMIC;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            update_function     = &dynamic_update;
        }
        else
        {
            desc.Usage          = D3D11_USAGE_DEFAULT;
            desc.CPUAccessFlags = 0;
            update_function     = &static_update;
        }

        desc.ByteWidth           = size;
        desc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
        desc.MiscFlags           = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA init_data;
        D3D11_SUBRESOURCE_DATA* p_init_data = nullptr;

        if (data)
        {
            init_data.pSysMem          = data;
            init_data.SysMemPitch      = size;
            init_data.SysMemSlicePitch = 0;
            p_init_data                = &init_data;
        }

        return DXAPI->m_device->CreateBuffer(&desc, p_init_data, &m_buffer) == S_OK;
    }

    void D3D11_VertexBuffer::update(size_t offset, size_t size, const byte* data)
    {
        update_function(m_buffer, offset, size, data);
    }

    void D3D11_VertexBuffer::bind(byte stream_index, size_t stride, size_t offset)
    {
        UINT d3d11_offset = static_cast<UINT>(offset);
        UINT d3d11_stride = static_cast<UINT>(stride);
        DXAPI->m_context->IASetVertexBuffers(stream_index, 1, &m_buffer, &d3d11_stride, &d3d11_offset);
    }

    D3D11_VertexBuffer::~D3D11_VertexBuffer()
    {
        d3d11_release(m_buffer);
    }

    RHI_VertexBuffer* D3D11::create_vertex_buffer(size_t size, const byte* data, RHIBufferType type)
    {
        D3D11_VertexBuffer* buffer = new D3D11_VertexBuffer();
        if (!buffer->init(size, data, type))
        {
            delete buffer;
            buffer = nullptr;
        }
        return buffer;
    }
}// namespace Engine
