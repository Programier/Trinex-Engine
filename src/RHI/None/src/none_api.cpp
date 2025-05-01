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

	struct NoneFence : public RHI_DefaultDestroyable<RHI_Fence> {
		bool is_signaled() override { return true; }
		void reset() override {}
	};


	struct NoneSampler : public RHI_DefaultDestroyable<RHI_Sampler> {
		void bind(BindLocation location) override {}
	};

	struct NoneRTV : public RHI_RenderTargetView {
		void clear(const LinearColor& color) override {}
		void blit(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override {}
	};

	struct NoneDSV : public RHI_DepthStencilView {
		void clear(float, byte) override {}
		void blit(RHI_DepthStencilView* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override {}
	};

	struct NoneTexture2D : public RHI_DefaultDestroyable<RHI_Texture2D> {
		RHI_RenderTargetView* as_rtv() override { return rhi_default<NoneRTV>(); }
		RHI_DepthStencilView* as_dsv() override { return rhi_default<NoneDSV>(); }
		RHI_ShaderResourceView* as_srv() override { return rhi_default<RHI_ShaderResourceView>(); }
		RHI_UnorderedAccessView* as_uav() override { return rhi_default<RHI_UnorderedAccessView>(); }
		void update(byte mip, const Rect2D& rect, const byte* data, size_t data_size) override {}
	};

	struct NoneShader : public RHI_DefaultDestroyable<RHI_Shader> {
	};

	struct NonePipeline : public RHI_DefaultDestroyable<RHI_Pipeline> {
		void bind() override {}
	};

	struct NoneBuffer : public RHI_DefaultDestroyable<RHI_Buffer> {
		byte* map() override { return nullptr; }
		void unmap() override {}
		void update(size_t offset, size_t size, const byte* data) override {}

		RHI_ShaderResourceView* as_srv() override { return rhi_default<RHI_ShaderResourceView>(); }
		RHI_UnorderedAccessView* as_uav() override { return rhi_default<RHI_UnorderedAccessView>(); }
	};

	struct NoneViewport : public RHI_DefaultDestroyable<RHI_Viewport> {
		void present() override {}

		void vsync(bool flag) override {}

		void on_resize(const Size2D& new_size) override {}

		void on_orientation_changed(Orientation orientation) override {}

		void bind() override {}

		void blit_target(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
		                 SamplerFilter filter) override
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

	NoneApi& NoneApi::viewport(const ViewPort& viewport)
	{
		return *this;
	}

	ViewPort NoneApi::viewport()
	{
		return {};
	}

	NoneApi& NoneApi::scissor(const Scissor& scissor)
	{
		return *this;
	}

	Scissor NoneApi::scissor()
	{
		return {};
	}

	RHI_Fence* NoneApi::create_fence()
	{
		return new NoneFence();
	}

	RHI_Sampler* NoneApi::create_sampler(const SamplerInitializer*)
	{
		return new NoneSampler();
	}

	RHI_Texture2D* NoneApi::create_texture_2d(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags)
	{
		return new NoneTexture2D();
	}

	RHI_Shader* NoneApi::create_vertex_shader(const VertexShader* shader)
	{
		return new NoneShader();
	}

	RHI_Shader* NoneApi::create_tesselation_control_shader(const TessellationControlShader* shader)
	{
		return new NoneShader();
	}

	RHI_Shader* NoneApi::create_tesselation_shader(const TessellationShader* shader)
	{
		return new NoneShader();
	}

	RHI_Shader* NoneApi::create_geometry_shader(const GeometryShader* shader)
	{
		return new NoneShader();
	}

	RHI_Shader* NoneApi::create_fragment_shader(const FragmentShader* shader)
	{
		return new NoneShader();
	}

	RHI_Shader* NoneApi::create_compute_shader(const ComputeShader* shader)
	{
		return new NoneShader();
	}

	RHI_Pipeline* NoneApi::create_graphics_pipeline(const GraphicsPipeline* pipeline)
	{
		return new NonePipeline();
	}

	RHI_Pipeline* NoneApi::create_compute_pipeline(const ComputePipeline* pipeline)
	{
		return new NonePipeline();
	}

	RHI_Buffer* NoneApi::create_buffer(size_t size, const byte* data, BufferCreateFlags flags)
	{
		return new NoneBuffer();
	}

	RHI_Viewport* NoneApi::create_viewport(WindowRenderViewport* viewport, bool vsync)
	{
		return new NoneViewport();
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


	NoneApi& NoneApi::bind_srv(RHI_ShaderResourceView* view, byte slot, RHI_Sampler* sampler)
	{
		return *this;
	}

	NoneApi& NoneApi::bind_uav(RHI_UnorderedAccessView* view, byte slot)
	{
		return *this;
	}

	NoneApi& NoneApi::update_scalar_parameter(const void* data, size_t size, size_t offset, BindingIndex buffer_index)
	{
		return *this;
	}

	NoneApi& NoneApi::push_debug_stage(const char* stage, const LinearColor& color)
	{
		return *this;
	}

	NoneApi& NoneApi::pop_debug_stage()
	{
		return *this;
	}
}// namespace Engine
