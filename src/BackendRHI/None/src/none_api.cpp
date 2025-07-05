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
			NoneApi::m_instance->info.struct_instance = static_struct_instance();
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

	struct NoneFence : public NoneApiDestroyable<RHI_Fence> {
		bool is_signaled() override { return true; }
		void reset() override {}
	};


	struct NoneSampler : public NoneApiDestroyable<RHI_Sampler> {
	};

	struct NoneRTV : public RHI_RenderTargetView {
		void clear(const LinearColor& color) override {}
		void blit(RHI_RenderTargetView* surface, const RHIRect& src_rect, const RHIRect& dst_rect,
		          RHISamplerFilter filter) override
		{}
	};

	struct NoneDSV : public RHI_DepthStencilView {
		void clear(float, byte) override {}
		void blit(RHI_DepthStencilView* surface, const RHIRect& src_rect, const RHIRect& dst_rect,
		          RHISamplerFilter filter) override
		{}
	};

	struct NoneTexture : public NoneApiDestroyable<RHI_Texture> {
		RHI_RenderTargetView* as_rtv(RHITextureDescRTV desc) override { return rhi_default<NoneRTV>(); }
		RHI_DepthStencilView* as_dsv(RHITextureDescDSV desc) override { return rhi_default<NoneDSV>(); }
		RHI_ShaderResourceView* as_srv(RHITextureDescSRV desc) override { return rhi_default<RHI_ShaderResourceView>(); }
		RHI_UnorderedAccessView* as_uav(RHITextureDescUAV desc) override { return rhi_default<RHI_UnorderedAccessView>(); }

		byte* map(RHIMappingAccess access, const RHIMappingRange* range) override { return nullptr; }
		void unmap(const RHIMappingRange* range) override {}
	};

	struct NoneShader : public NoneApiDestroyable<RHI_Shader> {
	};

	struct NonePipeline : public NoneApiDestroyable<RHI_Pipeline> {
		void bind() override {}
	};

	struct NoneBuffer : public NoneApiDestroyable<RHI_Buffer> {
		byte* map() override { return nullptr; }
		void unmap() override {}

		RHI_ShaderResourceView* as_srv() override { return rhi_default<RHI_ShaderResourceView>(); }
		RHI_UnorderedAccessView* as_uav() override { return rhi_default<RHI_UnorderedAccessView>(); }
	};

	struct NoneViewport : public NoneApiDestroyable<RHI_Viewport> {
		void present() override {}
		void vsync(bool flag) override {}
		void on_resize(const Size2D& new_size) override {}
		void bind() override {}

		void blit_target(RHI_RenderTargetView* surface, const RHIRect& src_rect, const RHIRect& dst_rect,
		                 RHISamplerFilter filter) override
		{}

		void clear_color(const LinearColor& color) override {}
	};


	NoneApi& NoneApi::initialize(Window* window)
	{
		return *this;
	}

	void* NoneApi::context()
	{
		return nullptr;
	}

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

	NoneApi& NoneApi::dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
	{
		return *this;
	}

	NoneApi& NoneApi::signal_fence(RHI_Fence* fence)
	{
		return *this;
	}

	NoneApi& NoneApi::submit()
	{
		return *this;
	}

	NoneApi& NoneApi::bind_render_target(RHI_RenderTargetView* rt1, RHI_RenderTargetView* rt2, RHI_RenderTargetView* rt3,
	                                     RHI_RenderTargetView* rt4, RHI_DepthStencilView* depth_stencil)
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

	RHI_Fence* NoneApi::create_fence()
	{
		return new NoneFence();
	}

	RHI_Sampler* NoneApi::create_sampler(const RHISamplerInitializer*)
	{
		return new NoneSampler();
	}

	RHI_Texture* NoneApi::create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, uint32_t mips,
	                                     RHITextureCreateFlags flags)
	{
		return new NoneTexture();
	}

	RHI_Shader* NoneApi::create_vertex_shader(const byte* source, size_t size, const RHIVertexAttribute* attributes,
	                                          size_t attributes_count)
	{
		return new NoneShader();
	}

	RHI_Shader* NoneApi::create_tesselation_control_shader(const byte* source, size_t size)
	{
		return new NoneShader();
	}

	RHI_Shader* NoneApi::create_tesselation_shader(const byte* source, size_t size)
	{
		return new NoneShader();
	}

	RHI_Shader* NoneApi::create_geometry_shader(const byte* source, size_t size)
	{
		return new NoneShader();
	}

	RHI_Shader* NoneApi::create_fragment_shader(const byte* source, size_t size)
	{
		return new NoneShader();
	}

	RHI_Shader* NoneApi::create_compute_shader(const byte* source, size_t size)
	{
		return new NoneShader();
	}

	RHI_Pipeline* NoneApi::create_graphics_pipeline(const RHIGraphicsPipelineInitializer* pipeline)
	{
		return new NonePipeline();
	}

	RHI_Pipeline* NoneApi::create_compute_pipeline(const RHIComputePipelineInitializer* pipeline)
	{
		return new NonePipeline();
	}

	RHI_Buffer* NoneApi::create_buffer(size_t size, const byte* data, RHIBufferCreateFlags flags)
	{
		return new NoneBuffer();
	}

	RHI_Viewport* NoneApi::create_viewport(WindowRenderViewport* viewport, bool vsync)
	{
		return new NoneViewport();
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

	NoneApi& NoneApi::bind_vertex_buffer(RHI_Buffer* buffer, size_t byte_offset, uint16_t stride, byte stream)
	{
		return *this;
	}

	NoneApi& NoneApi::bind_index_buffer(RHI_Buffer* buffer, RHIIndexFormat format)
	{
		return *this;
	}

	NoneApi& NoneApi::bind_uniform_buffer(RHI_Buffer* buffer, byte slot)
	{
		return *this;
	}

	NoneApi& NoneApi::bind_srv(RHI_ShaderResourceView* view, byte slot)
	{
		return *this;
	}

	NoneApi& NoneApi::bind_uav(RHI_UnorderedAccessView* view, byte slot)
	{
		return *this;
	}

	NoneApi& NoneApi::bind_sampler(RHI_Sampler* sampler, byte slot)
	{
		return *this;
	}

	NoneApi& NoneApi::update_buffer(RHI_Buffer* buffer, size_t offset, size_t size, const byte* data)
	{
		return *this;
	}

	NoneApi& NoneApi::update_texture(RHI_Texture*, const RHITextureUpdateDesc& desc)
	{
		return *this;
	}

	NoneApi& NoneApi::copy_buffer_to_buffer(RHI_Buffer* src, RHI_Buffer* dst, size_t size, size_t src_offset, size_t dst_offset)
	{
		return *this;
	}

	NoneApi& NoneApi::barrier(RHI_Texture* texture, RHIAccess dst_access)
	{
		return *this;
	}

	NoneApi& NoneApi::barrier(RHI_Buffer* buffer, RHIAccess dst_access)
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
