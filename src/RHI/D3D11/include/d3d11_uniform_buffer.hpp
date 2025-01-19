#pragma once
#include <Core/etl/vector.hpp>
#include <d3d11_buffer.hpp>

namespace Engine
{
	class D3D11_LocalUniforms
	{
		D3D11_UniformBuffer m_uniform_buffer;
		size_t m_current_size = 0;

	public:
		Vector<byte> m_shadow_data;
		D3D11_LocalUniforms& create(size_t size = 1024);
		D3D11_LocalUniforms& update(const void* data, size_t size, size_t offset);
		D3D11_LocalUniforms& release();
		D3D11_LocalUniforms& bind();
	};
}// namespace Engine
