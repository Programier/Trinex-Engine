#pragma once
#include <Graphics/shader_parameters.hpp>
#include <d3d11.h>

namespace Engine
{
	class D3D11_UniformBuffer
	{
	protected:
		ID3D11Buffer* m_buffer = nullptr;
		size_t m_buffer_size   = 0;

		D3D11_UniformBuffer& create(size_t size);
		D3D11_UniformBuffer& copy(size_t size, const void* data);
		D3D11_UniformBuffer& bind(BindingIndex index);
		D3D11_UniformBuffer& destroy();
	};

	class D3D11_GlobalUniforms : public D3D11_UniformBuffer
	{
	public:
		Vector<GlobalShaderParameters> m_stack;
		static constexpr size_t block_size = sizeof(GlobalShaderParameters);

		D3D11_GlobalUniforms& initialize();
		D3D11_GlobalUniforms& release();

		D3D11_GlobalUniforms& bind();

		D3D11_GlobalUniforms& push_global_params(const GlobalShaderParameters& params);
		D3D11_GlobalUniforms& update();
		D3D11_GlobalUniforms& pop_global_params();
	};

	class D3D11_LocalUniforms : public D3D11_UniformBuffer
	{
	public:
		Vector<byte> m_shadow_data;

		D3D11_LocalUniforms& initialize();
		D3D11_LocalUniforms& release();
		D3D11_LocalUniforms& update(const void* data, size_t size, size_t offset);
		D3D11_LocalUniforms& bind();
	};
}// namespace Engine
