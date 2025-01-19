#include <cstring>
#include <d3d11_api.hpp>
#include <d3d11_buffer.hpp>
#include <d3d11_pipeline.hpp>

namespace Engine
{
	void static update_buffer_internal(ID3D11Buffer* buffer, size_t offset, size_t size, const byte* data)
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
	static bool create_buffer(ID3D11Buffer*& out_buffer, size_t size, const byte* data, UINT bind_flags)
	{
		D3D11_BUFFER_DESC desc = {};

		desc.Usage               = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags      = 0;
		desc.ByteWidth           = size;
		desc.BindFlags           = bind_flags;
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

		return DXAPI->m_device->CreateBuffer(&desc, p_init_data, &out_buffer) == S_OK;
	}

	bool D3D11_VertexBuffer::init(size_t size, const byte* data, RHIBufferType type)
	{
		return create_buffer(m_buffer, size, data, D3D11_BIND_VERTEX_BUFFER);
	}

	void D3D11_VertexBuffer::update(size_t offset, size_t size, const byte* data)
	{
		update_buffer_internal(m_buffer, offset, size, data);
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

	bool D3D11_IndexBuffer::init(size_t size, const byte* data, RHIBufferType type, IndexBufferFormat format)
	{
		m_format = format == IndexBufferFormat::UInt32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		return create_buffer(m_buffer, size, data, D3D11_BIND_INDEX_BUFFER);
	}

	void D3D11_IndexBuffer::bind(size_t offset)
	{
		DXAPI->m_context->IASetIndexBuffer(m_buffer, m_format, offset);
	}

	void D3D11_IndexBuffer::update(size_t offset, size_t size, const byte* data)
	{
		update_buffer_internal(m_buffer, offset, size, data);
	}

	D3D11_IndexBuffer::~D3D11_IndexBuffer()
	{
		d3d11_release(m_buffer);
	}

	bool D3D11_UniformBuffer::init(size_t size, const byte* data, RHIBufferType type)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage             = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth         = size;
		desc.BindFlags         = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags    = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = DXAPI->m_device->CreateBuffer(&desc, nullptr, &m_buffer);
		return hr == S_OK;
	}

	void D3D11_UniformBuffer::bind(BindingIndex location)
	{
		auto& pipeline = DXAPI->m_state.pipeline;

		if (pipeline == nullptr)
			return;

		if (pipeline->m_vertex_shader)
			DXAPI->m_context->VSSetConstantBuffers(location, 1, &m_buffer);
		if (pipeline->m_tsc_shader)
			DXAPI->m_context->HSSetConstantBuffers(location, 1, &m_buffer);
		if (pipeline->m_ts_shader)
			DXAPI->m_context->DSSetConstantBuffers(location, 1, &m_buffer);
		if (pipeline->m_geometry_shader)
			DXAPI->m_context->GSSetConstantBuffers(location, 1, &m_buffer);
		if (pipeline->m_fragment_shader)
			DXAPI->m_context->PSSetConstantBuffers(location, 1, &m_buffer);
	}

	void D3D11_UniformBuffer::update(size_t offset, size_t size, const byte* data)
	{
		D3D11_MAPPED_SUBRESOURCE mapped_resource{};
		HRESULT hr = DXAPI->m_context->Map(m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);

		if (hr == S_OK)
		{
			std::memcpy(mapped_resource.pData, data, size);
			DXAPI->m_context->Unmap(m_buffer, 0);
		}
	}

	D3D11_UniformBuffer::~D3D11_UniformBuffer()
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

	RHI_IndexBuffer* D3D11::create_index_buffer(size_t size, const byte* data, IndexBufferFormat format, RHIBufferType type)
	{
		D3D11_IndexBuffer* buffer = new D3D11_IndexBuffer();
		if (!buffer->init(size, data, type, format))
		{
			delete buffer;
			buffer = nullptr;
		}

		return buffer;
	}

	RHI_SSBO* D3D11::create_ssbo(size_t size, const byte* data, RHIBufferType type)
	{
		return NoneApi::create_ssbo(size, data, type);
	}

	RHI_UniformBuffer* D3D11::create_uniform_buffer(size_t size, const byte* data, RHIBufferType type)
	{
		D3D11_UniformBuffer* buffer = new D3D11_UniformBuffer();
		if (!buffer->init(size, data, type))
		{
			delete buffer;
			buffer = nullptr;
		}

		return buffer;
	}
}// namespace Engine
