#pragma once
#include <Core/etl/map.hpp>
#include <Core/render_resource_ptr.hpp>
#include <Graphics/rhi.hpp>
#include <dx12_destroyable.hpp>
#include <dx12_headers.hpp>

namespace Engine
{
	class D3D12_State;

	class D3D12_Shader : public D3D12_DeferredDestroyable<RHIShader>
	{
	public:
		byte* const data;
		const size_t size;

		D3D12_Shader(const byte* data, size_t size);
		~D3D12_Shader();
	};

	class D3D12_VertexShader : public D3D12_Shader
	{
	public:
		D3D12_INPUT_ELEMENT_DESC* input_elements;
		size_t input_elements_count;

		D3D12_VertexShader(const byte* data, size_t size, const VertexAttribute* attributes, size_t attributes_count);
		~D3D12_VertexShader();
	};

	class D3D12_GraphicsPipeline : public D3D12_DeferredDestroyable<RHIPipeline>
	{
	private:
		struct Key {
			DXGI_FORMAT formats[5] = {};

			Key& init(D3D12_State* state);
			bool operator<(const Key& key) const;
		};

		TreeMap<Key, ComPtr<ID3D12PipelineState>> m_pipelines;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_desc = {};
		RenderResourcePtr<D3D12_Shader> m_shaders[5];
		D3D12_PRIMITIVE_TOPOLOGY m_topology;

		D3D12_GraphicsPipeline& bind_pipeline(const ComPtr<ID3D12PipelineState>& pipeline);

	public:
		D3D12_GraphicsPipeline(const GraphicsPipeline* pipeline);
		D3D12_GraphicsPipeline& flush(D3D12_State* state);
		void bind() override;
	};
}// namespace Engine
