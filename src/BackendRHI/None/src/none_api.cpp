#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/struct.hpp>
#include <none_api.hpp>

namespace Engine
{
	template<typename T>
	T* rhi_default()
	{
		static T t;
		return &t;
	}

	template<typename T>
	class NoneApiDestroyable : public T
	{
	public:
		using T::T;
		void destroy() override { delete this; }
	};

	NoneApi* NoneApi::m_instance = nullptr;

	NoneApi* NoneApi::static_constructor()
	{
		if (NoneApi::m_instance == nullptr)
		{
			NoneApi::m_instance                       = new NoneApi();
			NoneApi::m_instance->info.name            = "None";
			NoneApi::m_instance->info.renderer        = "None";
			NoneApi::m_instance->info.struct_instance = static_reflection();
		}
		return NoneApi::m_instance;
	}

	void NoneApi::static_destructor(NoneApi* api)
	{
		if (api == m_instance)
		{
			delete api;
			m_instance = nullptr;
		}
	}

	namespace TRINEX_RHI
	{
		using NONE = NoneApi;
	}

	trinex_implement_struct_default_init(Engine::TRINEX_RHI::NONE, 0);

	struct NoneTimestamp : public NoneApiDestroyable<RHITimestamp> {
		bool is_ready() override { return true; }
		float milliseconds() override { return 0.f; }
	};

	struct NonePipelineStatistics : public NoneApiDestroyable<RHIPipelineStatistics> {
		bool is_ready() override { return true; }

		uint64_t vertices() override { return 0; }
		uint64_t primitives() override { return 0; }
		uint64_t geometry_shader_primitives() override { return 0; }
		uint64_t clipping_primitives() override { return 0; }

		uint64_t vertex_shader_invocations() override { return 0; }
		uint64_t tessellation_control_shader_invocations() override { return 0; }
		uint64_t tesselation_shader_invocations() override { return 0; }
		uint64_t geometry_shader_invocations() override { return 0; }
		uint64_t clipping_invocations() override { return 0; }
		uint64_t fragment_shader_invocations() override { return 0; }
	};

	struct NoneFence : public NoneApiDestroyable<RHIFence> {
		bool is_signaled() override { return true; }
		void reset() override {}
	};


	struct NoneSampler : public NoneApiDestroyable<RHISampler> {
	};

	struct NoneRTV : public RHIRenderTargetView {
		void clear(const LinearColor& color) override {}
		void clear_uint(const Vector4u& value) override {}
		void clear_sint(const Vector4i& value) override {}
	};

	struct NoneDSV : public RHIDepthStencilView {
		void clear(float, byte) override {}
	};

	struct NoneTexture : public NoneApiDestroyable<RHITexture> {
		RHIRenderTargetView* as_rtv(RHITextureDescRTV* desc) override { return rhi_default<NoneRTV>(); }
		RHIDepthStencilView* as_dsv(RHITextureDescDSV* desc) override { return rhi_default<NoneDSV>(); }
		RHIShaderResourceView* as_srv(RHITextureDescSRV* desc) override { return rhi_default<RHIShaderResourceView>(); }
		RHIUnorderedAccessView* as_uav(RHITextureDescUAV* desc) override { return rhi_default<RHIUnorderedAccessView>(); }
	};

	struct NoneShader : public NoneApiDestroyable<RHIShader> {
	};

	struct NonePipeline : public NoneApiDestroyable<RHIPipeline> {
		void bind() override {}
	};

	struct NoneBuffer : public NoneApiDestroyable<RHIBuffer> {
		byte* map() override { return nullptr; }
		void unmap() override {}

		RHIShaderResourceView* as_srv(uint32_t offset, uint32_t size) override { return rhi_default<RHIShaderResourceView>(); }
		RHIUnorderedAccessView* as_uav(uint32_t offset, uint32_t size) override { return rhi_default<RHIUnorderedAccessView>(); }
	};

	struct NoneSwapchain : public NoneApiDestroyable<RHISwapchain> {
		void vsync(bool flag) override {}
		void resize(const Vector2u& new_size) override {}
		RHIRenderTargetView* as_rtv() override { return rhi_default<NoneRTV>(); }
	};

	NoneApi& NoneApi::draw(size_t vertex_count, size_t vertices_offset)
	{
		return *this;
	}

	NoneApi& NoneApi::draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset)
	{
		return *this;
	}

	NoneApi& NoneApi::draw_instanced(size_t vertex_count, size_t vertex_offset, size_t instances)
	{
		return *this;
	}

	NoneApi& NoneApi::draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
	                                         size_t instances)
	{
		return *this;
	}

	NoneApi& NoneApi::draw_mesh(uint32_t x, uint32_t y, uint32_t z)
	{
		return *this;
	}

	NoneApi& NoneApi::dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
	{
		return *this;
	}

	NoneApi& NoneApi::signal_fence(RHIFence* fence)
	{
		return *this;
	}

	NoneApi& NoneApi::submit()
	{
		return *this;
	}

	NoneApi& NoneApi::bind_render_target(RHIRenderTargetView* rt1, RHIRenderTargetView* rt2, RHIRenderTargetView* rt3,
	                                     RHIRenderTargetView* rt4, RHIDepthStencilView* depth_stencil)
	{
		return *this;
	}

	NoneApi& NoneApi::viewport(const RHIViewport& viewport)
	{
		return *this;
	}

	NoneApi& NoneApi::scissor(const RHIScissors& scissor)
	{
		return *this;
	}

	RHITimestamp* NoneApi::create_timestamp()
	{
		return new NoneTimestamp();
	}

	RHIPipelineStatistics* NoneApi::create_pipeline_statistics()
	{
		return new NonePipelineStatistics();
	}

	RHIFence* NoneApi::create_fence()
	{
		return new NoneFence();
	}

	RHISampler* NoneApi::create_sampler(const RHISamplerInitializer*)
	{
		return new NoneSampler();
	}

	RHITexture* NoneApi::create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, uint32_t mips,
	                                    RHITextureCreateFlags flags)
	{
		return new NoneTexture();
	}

	RHIShader* NoneApi::create_shader(const byte* source, size_t size)
	{
		return new NoneShader();
	}

	RHIPipeline* NoneApi::create_graphics_pipeline(const RHIGraphicsPipelineInitializer* pipeline)
	{
		return new NonePipeline();
	}

	RHIPipeline* NoneApi::create_mesh_pipeline(const RHIMeshPipelineInitializer* pipeline)
	{
		return new NonePipeline();
	}

	RHIPipeline* NoneApi::create_compute_pipeline(const RHIComputePipelineInitializer* pipeline)
	{
		return new NonePipeline();
	}

	RHIBuffer* NoneApi::create_buffer(size_t size, const byte* data, RHIBufferCreateFlags flags)
	{
		return new NoneBuffer();
	}

	RHISwapchain* NoneApi::create_swapchain(Window* window, bool vsync)
	{
		return new NoneSwapchain();
	}

	NoneApi& NoneApi::primitive_topology(RHIPrimitiveTopology topology)
	{
		return *this;
	}

	NoneApi& NoneApi::polygon_mode(RHIPolygonMode mode)
	{
		return *this;
	}

	NoneApi& NoneApi::cull_mode(RHICullMode mode)
	{
		return *this;
	}

	NoneApi& NoneApi::front_face(RHIFrontFace face)
	{
		return *this;
	}

	NoneApi& NoneApi::write_mask(RHIColorComponent mask)
	{
		return *this;
	}

	NoneApi& NoneApi::bind_vertex_buffer(RHIBuffer* buffer, size_t byte_offset, uint16_t stride, byte stream)
	{
		return *this;
	}

	NoneApi& NoneApi::bind_index_buffer(RHIBuffer* buffer, RHIIndexFormat format)
	{
		return *this;
	}

	NoneApi& NoneApi::bind_uniform_buffer(RHIBuffer* buffer, byte slot)
	{
		return *this;
	}

	NoneApi& NoneApi::bind_srv(RHIShaderResourceView* view, byte slot)
	{
		return *this;
	}

	NoneApi& NoneApi::bind_uav(RHIUnorderedAccessView* view, byte slot)
	{
		return *this;
	}

	NoneApi& NoneApi::bind_sampler(RHISampler* sampler, byte slot)
	{
		return *this;
	}

	NoneApi& NoneApi::update_buffer(RHIBuffer* buffer, size_t offset, size_t size, const byte* data)
	{
		return *this;
	}

	NoneApi& NoneApi::update_texture(RHITexture* texture, const RHITextureRegion& region, const void* data, size_t size,
	                                 size_t buffer_width, size_t buffer_height)
	{
		return *this;
	}

	NoneApi& NoneApi::copy_buffer_to_buffer(RHIBuffer* src, RHIBuffer* dst, size_t size, size_t src_offset, size_t dst_offset)
	{
		return *this;
	}

	NoneApi& NoneApi::copy_texture_to_buffer(RHITexture* texture, uint8_t mip_level, uint16_t array_slice, const Vector3u& offset,
	                                         const Vector3u& extent, RHIBuffer* buffer, size_t buffer_offset)
	{
		return *this;
	}

	NoneApi& NoneApi::copy_buffer_to_texture(RHIBuffer* buffer, size_t buffer_offset, RHITexture* texture, uint8_t mip_level,
	                                         uint16_t array_slice, const Vector3u& offset, const Vector3u& extent)
	{
		return *this;
	}

	NoneApi& NoneApi::copy_texture_to_texture(RHITexture* src, const RHITextureRegion& src_region, RHITexture* dst,
	                                          const RHITextureRegion& dst_region)
	{
		return *this;
	}

	NoneApi& NoneApi::barrier(RHITexture* texture, RHIAccess dst_access)
	{
		return *this;
	}

	NoneApi& NoneApi::barrier(RHIBuffer* buffer, RHIAccess dst_access)
	{
		return *this;
	}

	NoneApi& NoneApi::begin_timestamp(RHITimestamp* timestamp)
	{
		return *this;
	}

	NoneApi& NoneApi::end_timestamp(RHITimestamp* timestamp)
	{
		return *this;
	}

	NoneApi& NoneApi::begin_statistics(RHIPipelineStatistics* stats)
	{
		return *this;
	}

	NoneApi& NoneApi::end_statistics(RHIPipelineStatistics* stats)
	{
		return *this;
	}

	NoneApi& NoneApi::present(RHISwapchain* swapchain)
	{
		return *this;
	}

	NoneApi& NoneApi::update_scalar_parameter(const void* data, size_t size, size_t offset, BindingIndex buffer_index)
	{
		return *this;
	}

	NoneApi& NoneApi::push_debug_stage(const char* stage)
	{
		return *this;
	}

	NoneApi& NoneApi::pop_debug_stage()
	{
		return *this;
	}
}// namespace Engine
