#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Graphics/pipeline.hpp>
#include <d3d11_api.hpp>
#include <d3d11_pipeline.hpp>
#include <d3d11_uniform_buffer.hpp>

namespace Engine
{
	D3D11_UniformBuffer& D3D11_UniformBuffer::create(size_t size)
	{
		destroy();
		m_buffer_size = size;

		D3D11_BUFFER_DESC desc = {};
		desc.Usage             = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth         = size;
		desc.BindFlags         = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags    = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = DXAPI->m_device->CreateBuffer(&desc, nullptr, &m_buffer);

		if (hr != S_OK)
		{
			throw EngineException("Failed to create uniform buffer");
		}
		return *this;
	}

	D3D11_UniformBuffer& D3D11_UniformBuffer::destroy()
	{
		d3d11_release(m_buffer);
		m_buffer_size = 0;
		return *this;
	}

	D3D11_UniformBuffer& D3D11_UniformBuffer::copy(size_t size, const void* data)
	{
		D3D11_MAPPED_SUBRESOURCE mapped_resource{};
		HRESULT hr = DXAPI->m_context->Map(m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);

		if (hr == S_OK)
		{
			std::memcpy(mapped_resource.pData, data, size);
			DXAPI->m_context->Unmap(m_buffer, 0);
		}
		else
		{
			error_log("D3D11", "Failed to map global uniform buffer");
		}
		return *this;
	}

	D3D11_UniformBuffer& D3D11_UniformBuffer::bind(BindingIndex index)
	{
		auto& pipeline = DXAPI->m_state.pipeline;

		if (pipeline == nullptr)
			return *this;

		if (pipeline->m_vertex_shader)
			DXAPI->m_context->VSSetConstantBuffers(index, 1, &m_buffer);
		if (pipeline->m_tsc_shader)
			DXAPI->m_context->HSSetConstantBuffers(index, 1, &m_buffer);
		if (pipeline->m_ts_shader)
			DXAPI->m_context->DSSetConstantBuffers(index, 1, &m_buffer);
		if (pipeline->m_geometry_shader)
			DXAPI->m_context->GSSetConstantBuffers(index, 1, &m_buffer);
		if (pipeline->m_fragment_shader)
			DXAPI->m_context->PSSetConstantBuffers(index, 1, &m_buffer);
		return *this;
	}

	D3D11_GlobalUniforms& D3D11_GlobalUniforms::initialize()
	{
		create(block_size);
		return *this;
	}

	D3D11_GlobalUniforms& D3D11_GlobalUniforms::release()
	{
		destroy();
		return *this;
	}

	D3D11_GlobalUniforms& D3D11_GlobalUniforms::bind()
	{
		if (D3D11_Pipeline* pipeline = DXAPI->m_state.pipeline)
		{
			auto params = pipeline->m_engine_pipeline->global_parameters;

			if (params.has_parameters())
			{
				D3D11_UniformBuffer::bind(params.bind_index());
			}
		}
		return *this;
	}

	D3D11_GlobalUniforms& D3D11_GlobalUniforms::push_global_params(const GlobalShaderParameters& params)
	{
		m_stack.push_back(params);
		return update();
	}

	D3D11_GlobalUniforms& D3D11_GlobalUniforms::update()
	{
		if (!m_stack.empty())
		{
			auto& current = m_stack.back();
			copy(block_size, &current);
		}
		return *this;
	}

	D3D11_GlobalUniforms& D3D11_GlobalUniforms::pop_global_params()
	{
		if (!m_stack.empty())
		{
			m_stack.pop_back();
		}
		return update();
	}


	D3D11_LocalUniforms& D3D11_LocalUniforms::initialize()
	{
		create(1024);
		return *this;
	}

	D3D11_LocalUniforms& D3D11_LocalUniforms::release()
	{
		destroy();
		return *this;
	}

	D3D11_LocalUniforms& D3D11_LocalUniforms::update(const void* data, size_t size, size_t offset)
	{
		size_t max_size = offset + size;
		if (m_shadow_data.size() < max_size)
		{
			m_shadow_data.resize(max_size);
		}

		std::memcpy(m_shadow_data.data() + offset, data, size);
		return *this;
	}

	D3D11_LocalUniforms& D3D11_LocalUniforms::bind()
	{
		if (!m_shadow_data.empty())
		{
			if (D3D11_Pipeline* pipeline = DXAPI->m_state.pipeline)
			{
				auto locals = pipeline->m_engine_pipeline->local_parameters;
				if (locals.has_parameters())
				{
					size_t shadow_data_size = m_shadow_data.size();

					if (m_buffer_size < shadow_data_size)
					{
						create(shadow_data_size);
					}

					copy(shadow_data_size, m_shadow_data.data());
					D3D11_UniformBuffer::bind(locals.bind_index());
				}
			}

			m_shadow_data.clear();
		}
		return *this;
	}

	D3D11& D3D11::push_global_params(const GlobalShaderParameters& params)
	{
		m_global_uniform_buffer.push_global_params(params);
		return *this;
	}

	D3D11& D3D11::pop_global_params()
	{
		m_global_uniform_buffer.pop_global_params();
		return *this;
	}

	D3D11& D3D11::update_local_parameter(const void* data, size_t size, size_t offset)
	{
		m_local_unifor_buffer.update(data, size, offset);
		return *this;
	}
}// namespace Engine
