#include <Core/etl/allocator.hpp>
#include <Core/etl/vector.hpp>
#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <cstring>
#include <dx12_api.hpp>
#include <dx12_enums.hpp>
#include <dx12_pipeline.hpp>
#include <dx12_state.hpp>

namespace Engine
{
	D3D12_Shader::D3D12_Shader(const byte* src, size_t size) : data(ByteAllocator::allocate(size)), size(size)
	{
		std::memcpy(data, src, size);
	}

	D3D12_Shader::~D3D12_Shader()
	{
		ByteAllocator::deallocate(data);
	}

	D3D12_VertexShader::D3D12_VertexShader(const byte* data, size_t size, const VertexAttribute* attributes,
	                                       size_t attributes_count)
	    : D3D12_Shader(data, size), input_elements_count(attributes_count)
	{
		input_elements = Allocator<D3D12_INPUT_ELEMENT_DESC>::allocate(attributes_count);

		for (size_t i = 0; i < attributes_count; ++i)
		{
			D3D12_INPUT_ELEMENT_DESC& desc   = input_elements[i];
			const VertexAttribute& attribute = attributes[i];
			desc.SemanticName                = semantic_name(attribute.semantic);
			desc.SemanticIndex               = attribute.semantic_index;
			desc.Format                      = format_of(attribute.type);
			desc.InputSlot                   = attribute.stream_index;
			desc.AlignedByteOffset           = attribute.offset;
			desc.InputSlotClass              = attribute.rate == VertexAttributeInputRate::Vertex
			                                           ? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
			                                           : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
			desc.InstanceDataStepRate        = attribute.rate == VertexAttributeInputRate::Vertex ? 0 : 1;
		}
	}

	D3D12_VertexShader::~D3D12_VertexShader()
	{
		Allocator<D3D12_INPUT_ELEMENT_DESC>::deallocate(input_elements);
	}

	RHIShader* D3D12::create_vertex_shader(const byte* shader, size_t size, const VertexAttribute* attributes,
	                                        size_t attributes_count)
	{
		return allocate<D3D12_VertexShader>(shader, size, attributes, attributes_count);
	}

	RHIShader* D3D12::create_tesselation_control_shader(const byte* shader, size_t size)
	{
		return allocate<D3D12_Shader>(shader, size);
	}

	RHIShader* D3D12::create_tesselation_shader(const byte* shader, size_t size)
	{
		return allocate<D3D12_Shader>(shader, size);
	}

	RHIShader* D3D12::create_geometry_shader(const byte* shader, size_t size)
	{
		return allocate<D3D12_Shader>(shader, size);
	}

	RHIShader* D3D12::create_fragment_shader(const byte* shader, size_t size)
	{
		return allocate<D3D12_Shader>(shader, size);
	}

	RHIShader* D3D12::create_compute_shader(const byte* shader, size_t size)
	{
		return allocate<D3D12_Shader>(shader, size);
	}

	D3D12_GraphicsPipeline::Key& D3D12_GraphicsPipeline::Key::init(D3D12_State* state)
	{
		auto& rtv_state = state->rtv_state();

		for (uint_t i = 0; i < 4; ++i)
		{
			if (auto rtv = rtv_state.resource(i))
				formats[i] = rtv->format();
		}

		if (auto dsv = state->dsv_state().resource(0))
			formats[4] = dsv->format();

		return *this;
	}

	bool D3D12_GraphicsPipeline::Key::operator<(const Key& key) const
	{
		return std::memcmp(formats, key.formats, sizeof(key)) < 0;
	}

	static inline D3D12_Shader* parse_shader(Shader* shader)
	{
		if (shader)
		{
			RHIShader* result = shader->rhi_shader();
			result->add_reference();
			return static_cast<D3D12_Shader*>(result);
		}
		return nullptr;
	}

	static inline void copy_shader(D3D12_SHADER_BYTECODE& out, D3D12_Shader* shader)
	{
		if (shader)
		{
			out.pShaderBytecode = shader->data;
			out.BytecodeLength  = shader->size;
		}
	}

	static D3D12_DEPTH_STENCIL_DESC create_depth_stencil_description(const GraphicsPipeline* pipeline)
	{
		D3D12_DEPTH_STENCIL_DESC desc{};

		auto& depth   = pipeline->depth_test;
		auto& stencil = pipeline->stencil_test;

		desc.DepthEnable                  = depth.enable ? TRUE : FALSE;
		desc.DepthWriteMask               = depth.enable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc                    = comparison_func_of(depth.func);
		desc.StencilEnable                = stencil.enable ? TRUE : FALSE;
		desc.StencilReadMask              = static_cast<UINT8>(stencil.compare_mask);
		desc.StencilWriteMask             = static_cast<UINT8>(stencil.write_mask);
		desc.FrontFace.StencilFailOp      = stencil_op_of(stencil.fail);
		desc.FrontFace.StencilDepthFailOp = stencil_op_of(stencil.depth_fail);
		desc.FrontFace.StencilPassOp      = stencil_op_of(stencil.depth_pass);
		desc.FrontFace.StencilFunc        = comparison_func_of(stencil.compare);
		desc.BackFace                     = desc.FrontFace;

		return desc;
	}

	static D3D12_BLEND_DESC create_blend_description(const GraphicsPipeline* pipeline)
	{
		D3D12_BLEND_DESC desc{};
		auto& blend = pipeline->color_blending;

		desc.AlphaToCoverageEnable  = FALSE;
		desc.IndependentBlendEnable = FALSE;

		desc.RenderTarget[0].BlendEnable    = blend.enable ? TRUE : FALSE;
		desc.RenderTarget[0].SrcBlend       = blend_func_of(blend.src_color_func);
		desc.RenderTarget[0].DestBlend      = blend_func_of(blend.dst_color_func);
		desc.RenderTarget[0].SrcBlendAlpha  = blend_func_of(blend.src_alpha_func);
		desc.RenderTarget[0].DestBlendAlpha = blend_func_of(blend.dst_alpha_func);
		desc.RenderTarget[0].BlendOp        = blend_op_of(blend.color_op);
		desc.RenderTarget[0].BlendOpAlpha   = blend_op_of(blend.alpha_op);

		desc.RenderTarget[0].RenderTargetWriteMask = component_mask_of(blend.color_mask);

		constexpr size_t elements = ARRAY_SIZE(desc.RenderTarget);

		for (size_t i = 1; i < elements; ++i)
		{
			desc.RenderTarget[i] = desc.RenderTarget[0];
		}

		return desc;
	}

	static D3D12_RASTERIZER_DESC create_rasterizer_description(const GraphicsPipeline* pipeline)
	{
		D3D12_RASTERIZER_DESC desc{};
		auto& rasterizer           = pipeline->rasterizer;
		desc.FillMode              = fill_mode_of(rasterizer.polygon_mode);
		desc.CullMode              = cull_mode_of(rasterizer.cull_mode);
		desc.FrontCounterClockwise = rasterizer.front_face == FrontFace::CounterClockWise;
		desc.DepthBias             = 0.f;
		desc.DepthBiasClamp        = 0.f;
		desc.SlopeScaledDepthBias  = 0.f;
		desc.DepthClipEnable       = TRUE;
		desc.MultisampleEnable     = FALSE;
		desc.AntialiasedLineEnable = FALSE;
		return desc;
	}

	D3D12_GraphicsPipeline::D3D12_GraphicsPipeline(const GraphicsPipeline* pipeline)
	{
		m_desc.pRootSignature = D3D12::api()->root_signature();

		m_shaders[0] = parse_shader(pipeline->vertex_shader());
		m_shaders[1] = parse_shader(pipeline->tessellation_control_shader());
		m_shaders[2] = parse_shader(pipeline->tessellation_shader());
		m_shaders[3] = parse_shader(pipeline->geometry_shader());
		m_shaders[4] = parse_shader(pipeline->fragment_shader());

		copy_shader(m_desc.VS, m_shaders[0].get());
		copy_shader(m_desc.DS, m_shaders[1].get());
		copy_shader(m_desc.HS, m_shaders[2].get());
		copy_shader(m_desc.GS, m_shaders[3].get());
		copy_shader(m_desc.PS, m_shaders[4].get());

		m_desc.BlendState                     = create_blend_description(pipeline);
		m_desc.SampleMask                     = D3D12_DEFAULT_SAMPLE_MASK;
		m_desc.RasterizerState                = create_rasterizer_description(pipeline);
		m_desc.DepthStencilState              = create_depth_stencil_description(pipeline);
		m_desc.InputLayout.pInputElementDescs = static_cast<D3D12_VertexShader*>(m_shaders[0].get())->input_elements;
		m_desc.InputLayout.NumElements        = static_cast<D3D12_VertexShader*>(m_shaders[0].get())->input_elements_count;
		m_desc.PrimitiveTopologyType          = primitive_topology_type_of(pipeline->input_assembly.primitive_topology);
		m_desc.SampleDesc.Count               = 1;
		m_desc.SampleDesc.Quality             = 0;

		m_topology = primitive_topology_of(pipeline->input_assembly.primitive_topology);
	}

	D3D12_GraphicsPipeline& D3D12_GraphicsPipeline::bind_pipeline(const ComPtr<ID3D12PipelineState>& pipeline)
	{
		auto cmd_list = D3D12::api()->cmd_list();
		cmd_list->SetPipelineState(pipeline.Get());
		cmd_list->IASetPrimitiveTopology(m_topology);
		return *this;
	}

	D3D12_GraphicsPipeline& D3D12_GraphicsPipeline::flush(D3D12_State* state)
	{
		if (!state->pipeline_state().is_any_resource_dirty() && !state->rtv_state().is_any_resource_dirty() &&
		    !state->dsv_state().is_any_resource_dirty())
		{
			return *this;
		}

		Key key;
		key.init(state);

		auto it = m_pipelines.find(key);
		if (it != m_pipelines.end())
			return bind_pipeline(it->second);

		for (int i = 0; i < 4; ++i)
		{
			m_desc.RTVFormats[i] = key.formats[i];

			if (key.formats[i] != DXGI_FORMAT_UNKNOWN)
				m_desc.NumRenderTargets = i + 1;
		}

		m_desc.DSVFormat = key.formats[4];

		ComPtr<ID3D12PipelineState> pipeline;
		auto result = D3D12::api()->device()->CreateGraphicsPipelineState(&m_desc, IID_PPV_ARGS(&pipeline));
		trinex_always_check(result == S_OK, "Failed to create pipeline");
		m_pipelines[key] = pipeline;
		return bind_pipeline(pipeline);
	}

	void D3D12_GraphicsPipeline::bind()
	{
		D3D12::api()->state()->bind_pipeline(this);
	}

	RHIPipeline* D3D12::create_graphics_pipeline(const GraphicsPipeline* pipeline)
	{
		return allocate<D3D12_GraphicsPipeline>(pipeline);
	}

	RHIPipeline* D3D12::create_compute_pipeline(const ComputePipeline* pipeline)
	{
		return NoneApi::create_compute_pipeline(pipeline);
	}
}// namespace Engine
