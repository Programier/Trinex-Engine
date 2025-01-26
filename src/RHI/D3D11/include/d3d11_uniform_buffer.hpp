#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/vector.hpp>

namespace Engine
{
	class D3D11_UniformBufferManager
	{
		Vector<class D3D11_LocalUniformBuffer*> m_buffers;

	public:
		D3D11_UniformBufferManager& update(const void* data, size_t size, size_t offset, BindingIndex buffer);
		D3D11_UniformBufferManager& bind();
		~D3D11_UniformBufferManager();
	};
}// namespace Engine
