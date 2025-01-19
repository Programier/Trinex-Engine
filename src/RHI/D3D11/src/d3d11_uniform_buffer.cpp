#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Graphics/pipeline.hpp>
#include <d3d11_api.hpp>
#include <d3d11_pipeline.hpp>
#include <d3d11_uniform_buffer.hpp>

namespace Engine
{
	D3D11_LocalUniforms& D3D11_LocalUniforms::create(size_t size)
	{
		if (size > m_current_size)
		{
			release();
			m_uniform_buffer.init(size, nullptr, RHIBufferType::Dynamic);
			m_current_size = size;
		}
		return *this;
	}

	D3D11_LocalUniforms& D3D11_LocalUniforms::release()
	{
		d3d11_release(m_uniform_buffer.m_buffer);
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

					if (m_current_size < shadow_data_size)
					{
						create(shadow_data_size);
					}

					m_uniform_buffer.update(0, shadow_data_size, m_shadow_data.data());
					m_uniform_buffer.bind(locals.bind_index());
				}
			}

			m_shadow_data.clear();
		}
		return *this;
	}

	D3D11& D3D11::update_scalar_parameter(const void* data, size_t size, size_t offset)
	{
		m_local_unifor_buffer.update(data, size, offset);
		return *this;
	}
}// namespace Engine
