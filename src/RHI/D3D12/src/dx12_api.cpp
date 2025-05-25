#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Core/reflection/struct.hpp>
#include <dx12_api.hpp>
#include <dx12_buffer.hpp>
#include <dx12_command_allocator.hpp>
#include <dx12_config.hpp>
#include <dx12_descriptor.hpp>
#include <dx12_state.hpp>

namespace Engine
{
	D3D12* D3D12::s_d3d12 = nullptr;

	namespace TRINEX_RHI
	{
		using D3D12 = Engine::D3D12;
	}

	trinex_implement_struct_default_init(Engine::TRINEX_RHI::D3D12, 0);

	D3D12* D3D12::static_constructor()
	{
		if (s_d3d12 == nullptr)
		{
			allocate<D3D12>();
		}
		return s_d3d12;
	}

	void D3D12::static_destructor(D3D12* d3d12)
	{
		if (s_d3d12 == d3d12)
		{
			release(d3d12);
			s_d3d12 = nullptr;
		}
	}

	D3D12::D3D12()
	{
		s_d3d12              = this;
		info.name            = "D3D12";
		info.struct_instance = static_struct_instance();

		if (D3D12Config::enable_debug)
		{
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug))))
				m_debug->EnableDebugLayer();
		}

		HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**) &m_factory);
		trinex_always_check(result == S_OK, "Failed to create DXGI Factory");

		result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));
		trinex_always_check(result == S_OK, "Failed to create D3D12 Device");

		D3D12_COMMAND_QUEUE_DESC queue_desc = {};
		queue_desc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queue_desc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;

		result = m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_command_queue));
		trinex_always_check(result == S_OK, "Failed to create command queue");

		m_descriptor_manager    = allocate<D3D12DescritorManager>();
		m_command_allocator     = allocate<D3D12CommandAlloctor>();
		m_state                 = allocate<D3D12_State>();
		m_upload_buffer_manager = allocate<D3D12UploadBufferManager>();

		result = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator->allocator(), nullptr,
		                                     IID_PPV_ARGS(&m_cmd_list));
		trinex_always_check(result == S_OK, "Failed to create command list");

		compile_root_signature().rebind_descriptors();
	}

	D3D12::~D3D12()
	{
		// TODO: Wait for device before destroying objects
		release(m_upload_buffer_manager);
		release(m_state);
		release(m_command_allocator);
		release(m_descriptor_manager);
	}

	D3D12& D3D12::compile_root_signature()
	{
		D3D12_DESCRIPTOR_RANGE ranges[4] = {};

		ranges[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		ranges[0].NumDescriptors                    = s_max_uniform_buffers;
		ranges[0].BaseShaderRegister                = 0;
		ranges[0].RegisterSpace                     = 0;
		ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		ranges[1].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		ranges[1].NumDescriptors                    = s_max_srv;
		ranges[1].BaseShaderRegister                = 0;
		ranges[1].RegisterSpace                     = 0;
		ranges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		ranges[2].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		ranges[2].NumDescriptors                    = s_max_uav;
		ranges[2].BaseShaderRegister                = 0;
		ranges[2].RegisterSpace                     = 0;
		ranges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		ranges[3].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		ranges[3].NumDescriptors                    = s_max_samplers;
		ranges[3].BaseShaderRegister                = 0;
		ranges[3].RegisterSpace                     = 0;
		ranges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER root_parameters[4];

		for (uint_t i = 0; i < 4; ++i)
		{
			root_parameters[i].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			root_parameters[i].DescriptorTable.NumDescriptorRanges = 1;
			root_parameters[i].DescriptorTable.pDescriptorRanges   = &ranges[i];
			root_parameters[i].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;
		}

		D3D12_ROOT_SIGNATURE_DESC root_signature_desc{};
		root_signature_desc.NumParameters     = 4;
		root_signature_desc.pParameters       = root_parameters;
		root_signature_desc.NumStaticSamplers = 0;
		root_signature_desc.pStaticSamplers   = nullptr;
		root_signature_desc.Flags             = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ComPtr<ID3DBlob> bytecode;
		ComPtr<ID3DBlob> error;
		HRESULT hr = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &bytecode, &error);

		if (FAILED(hr))
		{
			if (error)
			{
				OutputDebugStringA((char*) error->GetBufferPointer());
			}
			throw EngineException("Failed to serialize root signature");
		}

		hr = D3D12::api()->device()->CreateRootSignature(0, bytecode->GetBufferPointer(), bytecode->GetBufferSize(),
		                                                 IID_PPV_ARGS(&m_root_signature));

		trinex_always_check(hr == S_OK, "Failed to create root signature");
		return *this;
	}

	D3D12& D3D12::rebind_descriptors()
	{
		m_cmd_list->SetGraphicsRootSignature(root_signature());
		return *this;
	}

	D3D12& D3D12::deferred_destroy(RHI_Object* object)
	{
		m_command_allocator->add_object_to_destroy(object);
		return *this;
	}

	D3D12& D3D12::submit()
	{
		m_cmd_list->Close();
		ID3D12CommandList* cmd_list[] = {m_cmd_list.Get()};
		m_command_queue->ExecuteCommandLists(1, cmd_list);
		m_command_allocator->submit();

		m_cmd_list->Reset(m_command_allocator->allocator(), nullptr);
		return rebind_descriptors();
	}

	D3D12& D3D12::viewport(const ViewPort& viewport)
	{
		D3D12_VIEWPORT vp = {};
		vp.Width          = viewport.size.x;
		vp.Height         = viewport.size.y;
		vp.TopLeftX       = viewport.pos.x;
		vp.TopLeftY       = viewport.pos.y;
		vp.MinDepth       = viewport.min_depth;
		vp.MaxDepth       = viewport.max_depth;
		m_cmd_list->RSSetViewports(1, &vp);
		return *this;
	}

	D3D12& D3D12::scissor(const Scissor& scissor)
	{
		D3D12_RECT rect = {};
		rect.left       = scissor.pos.x;
		rect.top        = scissor.pos.y;
		rect.right      = scissor.pos.x + scissor.size.x;
		rect.bottom     = scissor.pos.y + scissor.size.y;
		m_cmd_list->RSSetScissorRects(1, &rect);
		return *this;
	}

	D3D12& D3D12::bind_vertex_buffer(RHI_Buffer* buffer, size_t byte_offset, uint16_t stride, byte stream)
	{
		D3D12Buffer* vertex_buffer = static_cast<D3D12Buffer*>(buffer);
		D3D12_VERTEX_BUFFER_VIEW view;
		view.BufferLocation = vertex_buffer->virtual_address() + byte_offset;
		view.SizeInBytes    = vertex_buffer->size() - byte_offset;
		view.StrideInBytes  = stride;

		m_cmd_list->IASetVertexBuffers(stream, 1, &view);
		return *this;
	}

	D3D12& D3D12::bind_index_buffer(RHI_Buffer* buffer, RHIIndexFormat format)
	{
		D3D12Buffer* index_buffer = static_cast<D3D12Buffer*>(buffer);
		D3D12_INDEX_BUFFER_VIEW view;
		view.BufferLocation = index_buffer->virtual_address();
		view.SizeInBytes    = index_buffer->size();
		view.Format         = format == RHIIndexFormat::UInt16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		m_cmd_list->IASetIndexBuffer(&view);
		return *this;
	}

	D3D12& D3D12::bind_render_target(RHI_RenderTargetView* rt1, RHI_RenderTargetView* rt2, RHI_RenderTargetView* rt3,
	                                 RHI_RenderTargetView* rt4, RHI_DepthStencilView* depth_stencil)
	{
		state()->bind_render_target(static_cast<D3D12_RTV*>(rt1), static_cast<D3D12_RTV*>(rt2), static_cast<D3D12_RTV*>(rt3),
		                            static_cast<D3D12_RTV*>(rt4), static_cast<D3D12_DSV*>(depth_stencil));
		return *this;
	}

	D3D12& D3D12::draw(size_t vertex_count, size_t vertices_offset)
	{
		state()->flush_graphics();
		m_cmd_list->DrawInstanced(vertex_count, 1, vertices_offset, 0);
		return *this;
	}

	void d3d12_deferred_destroy(RHI_Object* object)
	{
		D3D12::api()->deferred_destroy(object);
	}
}// namespace Engine
