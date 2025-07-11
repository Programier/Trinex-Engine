#pragma once
#include <Core/types/color.hpp>
#include <RHI/enums.hpp>
#include <RHI/structures.hpp>

namespace Engine
{
	class WindowRenderViewport;
	class GraphicsPipeline;
	class ComputePipeline;

	struct RHISamplerInitializer;
	struct RHIGraphicsPipelineInitializer;
	struct RHIComputePipelineInitializer;

	struct LinearColor;

	namespace Refl
	{
		class Struct;
	}

	struct ENGINE_EXPORT RHI_Object {
	private:
		static void static_release_internal(RHI_Object* object);

	protected:
		size_t m_references;

	public:
		RHI_Object(size_t init_ref_count = 1);
		virtual void add_reference();
		virtual void release();
		virtual void destroy() = 0;
		size_t references() const;
		virtual ~RHI_Object();

		template<typename T>
		static inline void static_release(T* object)
		{
			if (object)
			{
				static_release_internal(object);
			}
		}

		template<typename T>
		T* as()
		{
			return static_cast<T*>(this);
		}

		template<typename T>
		const T* as() const
		{
			return static_cast<const T*>(this);
		}
	};

	struct ENGINE_EXPORT RHITimestamp : RHI_Object {
		virtual bool is_ready()      = 0;
		virtual float milliseconds() = 0;
	};

	struct ENGINE_EXPORT RHIPipelineStatistics : RHI_Object {
		virtual bool is_ready() = 0;

		virtual uint64_t vertices()                   = 0;
		virtual uint64_t primitives()                 = 0;
		virtual uint64_t geometry_shader_primitives() = 0;
		virtual uint64_t clipping_primitives()        = 0;

		virtual uint64_t vertex_shader_invocations()               = 0;
		virtual uint64_t tessellation_control_shader_invocations() = 0;
		virtual uint64_t tesselation_shader_invocations()          = 0;
		virtual uint64_t geometry_shader_invocations()             = 0;
		virtual uint64_t clipping_invocations()                    = 0;
		virtual uint64_t fragment_shader_invocations()             = 0;
	};

	struct ENGINE_EXPORT RHI_Fence : RHI_Object {
		virtual bool is_signaled() = 0;
		virtual void reset()       = 0;
	};

	struct ENGINE_EXPORT RHI_ShaderResourceView {
		virtual ~RHI_ShaderResourceView() {}
	};

	struct ENGINE_EXPORT RHI_UnorderedAccessView {
		virtual ~RHI_UnorderedAccessView() {}
	};

	struct ENGINE_EXPORT RHI_RenderTargetView {
		virtual void clear(const LinearColor& color)   = 0;
		virtual void clear_uint(const Vector4u& value) = 0;
		virtual void clear_sint(const Vector4i& value) = 0;

		virtual void blit(RHI_RenderTargetView* surface, const RHIRect& src_rect, const RHIRect& dst_rect,
		                  RHISamplerFilter filter) = 0;
		virtual ~RHI_RenderTargetView() {}
	};

	struct ENGINE_EXPORT RHI_DepthStencilView {
		virtual void clear(float depth, byte stencil) = 0;
		virtual void blit(RHI_DepthStencilView* surface, const RHIRect& src_rect, const RHIRect& dst_rect,
		                  RHISamplerFilter filter)    = 0;
		virtual ~RHI_DepthStencilView() {}
	};

	struct ENGINE_EXPORT RHI_Sampler : RHI_Object {
	};

	struct ENGINE_EXPORT RHI_Texture : RHI_Object {
		virtual RHI_RenderTargetView* as_rtv(RHITextureDescRTV desc = {})    = 0;
		virtual RHI_DepthStencilView* as_dsv(RHITextureDescDSV desc = {})    = 0;
		virtual RHI_ShaderResourceView* as_srv(RHITextureDescSRV desc = {})  = 0;
		virtual RHI_UnorderedAccessView* as_uav(RHITextureDescUAV desc = {}) = 0;
	};

	struct ENGINE_EXPORT RHI_Shader : RHI_Object {
	};

	struct ENGINE_EXPORT RHI_Pipeline : RHI_Object {
		virtual void bind() = 0;
	};

	struct ENGINE_EXPORT RHI_Buffer : RHI_Object {
		virtual byte* map()                       = 0;
		virtual void unmap()                      = 0;
		virtual RHI_ShaderResourceView* as_srv()  = 0;
		virtual RHI_UnorderedAccessView* as_uav() = 0;
	};

	struct ENGINE_EXPORT RHI_Viewport : RHI_Object {
		virtual void present() = 0;

		virtual void vsync(bool flag)                      = 0;
		virtual void on_resize(const Size2D& new_size)     = 0;
		virtual void bind()                                = 0;
		virtual void blit_target(RHI_RenderTargetView* surface, const RHIRect& src_rect, const RHIRect& dst_rect,
		                         RHISamplerFilter filter)  = 0;
		virtual void clear_color(const LinearColor& color) = 0;
	};

	struct ENGINE_EXPORT RHI {
		static constexpr size_t s_max_srv             = 32;
		static constexpr size_t s_max_samplers        = 32;
		static constexpr size_t s_max_uav             = 8;
		static constexpr size_t s_max_uniform_buffers = 8;
		static constexpr size_t s_max_vertex_buffers  = 8;

		struct Info {
			String name;
			String renderer;
			Refl::Struct* struct_instance = nullptr;
			Vector2f ndc_depth_range      = {0.f, 1.f};
		} info;

		virtual RHI& initialize(class Window* window) = 0;
		virtual void* context()                       = 0;

		virtual RHI& draw(size_t vertex_count, size_t vertices_offset)                                 = 0;
		virtual RHI& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) = 0;
		virtual RHI& draw_instanced(size_t vertex_count, size_t vertex_offset, size_t instances)       = 0;
		virtual RHI& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
		                                    size_t instances)                                          = 0;

		virtual RHI& dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) = 0;
		virtual RHI& signal_fence(RHI_Fence* fence)                                 = 0;
		virtual RHI& submit()                                                       = 0;

		virtual RHI& bind_render_target(RHI_RenderTargetView* rt1, RHI_RenderTargetView* rt2, RHI_RenderTargetView* rt3,
		                                RHI_RenderTargetView* rt4, RHI_DepthStencilView* depth_stencil) = 0;

		virtual RHI& viewport(const RHIViewport& viewport) = 0;
		virtual RHI& scissor(const RHIScissors& scissor)   = 0;

		virtual RHITimestamp* create_timestamp()                                                                      = 0;
		virtual RHIPipelineStatistics* create_pipeline_statistics()                                                   = 0;
		virtual RHI_Fence* create_fence()                                                                             = 0;
		virtual RHI_Sampler* create_sampler(const RHISamplerInitializer*)                                             = 0;
		virtual RHI_Texture* create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, uint32_t mips,
		                                    RHITextureCreateFlags flags)                                              = 0;
		virtual RHI_Shader* create_vertex_shader(const byte* shader, size_t size, const RHIVertexAttribute* attributes,
		                                         size_t attributes_count)                                             = 0;
		virtual RHI_Shader* create_tesselation_control_shader(const byte* shader, size_t size)                        = 0;
		virtual RHI_Shader* create_tesselation_shader(const byte* shader, size_t size)                                = 0;
		virtual RHI_Shader* create_geometry_shader(const byte* shader, size_t size)                                   = 0;
		virtual RHI_Shader* create_fragment_shader(const byte* shader, size_t size)                                   = 0;
		virtual RHI_Shader* create_compute_shader(const byte* shader, size_t size)                                    = 0;
		virtual RHI_Pipeline* create_graphics_pipeline(const RHIGraphicsPipelineInitializer* pipeline)                = 0;
		virtual RHI_Pipeline* create_compute_pipeline(const RHIComputePipelineInitializer* pipeline)                  = 0;
		virtual RHI_Buffer* create_buffer(size_t size, const byte* data, RHIBufferCreateFlags flags)                  = 0;
		virtual RHI_Viewport* create_viewport(WindowRenderViewport* viewport, bool vsync)                             = 0;
		virtual RHI& update_scalar_parameter(const void* data, size_t size, size_t offset, BindingIndex buffer_index) = 0;
		virtual RHI& push_debug_stage(const char* stage)                                                              = 0;
		virtual RHI& pop_debug_stage()                                                                                = 0;

		virtual RHI& update_buffer(RHI_Buffer* buffer, size_t offset, size_t size, const byte* data) = 0;
		virtual RHI& update_texture(RHI_Texture* texture, const RHITextureUpdateDesc& desc)          = 0;

		virtual RHI& copy_buffer_to_buffer(RHI_Buffer* src, RHI_Buffer* dst, size_t size, size_t src_offset,
		                                   size_t dst_offset) = 0;

		virtual RHI& copy_texture_to_buffer(RHI_Texture* texture, uint8_t mip_level, uint16_t array_slice, const Vector3u& offset,
		                                    const Vector3u& extent, RHI_Buffer* buffer, size_t buffer_offset) = 0;

		virtual RHI& copy_buffer_to_texture(RHI_Buffer* buffer, size_t buffer_offset, RHI_Texture* texture, uint8_t mip_level,
		                                    uint16_t array_slice, const Vector3u& offset, const Vector3u& extent) = 0;

		virtual RHI& primitive_topology(RHIPrimitiveTopology topology) = 0;
		virtual RHI& polygon_mode(RHIPolygonMode mode)                 = 0;
		virtual RHI& cull_mode(RHICullMode mode)                       = 0;
		virtual RHI& front_face(RHIFrontFace face)                     = 0;

		virtual RHI& bind_vertex_buffer(RHI_Buffer* buffer, size_t byte_offset, uint16_t stride, byte stream) = 0;
		virtual RHI& bind_index_buffer(RHI_Buffer* buffer, RHIIndexFormat format)                             = 0;
		virtual RHI& bind_uniform_buffer(RHI_Buffer* buffer, byte slot)                                       = 0;

		virtual RHI& bind_sampler(RHI_Sampler* sampler, byte slot)      = 0;
		virtual RHI& bind_srv(RHI_ShaderResourceView* view, byte slot)  = 0;
		virtual RHI& bind_uav(RHI_UnorderedAccessView* view, byte slot) = 0;

		virtual RHI& barrier(RHI_Texture* texture, RHIAccess access) = 0;
		virtual RHI& barrier(RHI_Buffer* buffer, RHIAccess access)   = 0;

		virtual RHI& begin_timestamp(RHITimestamp* timestamp) = 0;
		virtual RHI& end_timestamp(RHITimestamp* timestamp)   = 0;

		virtual RHI& begin_statistics(RHIPipelineStatistics* stats) = 0;
		virtual RHI& end_statistics(RHIPipelineStatistics* stats)   = 0;

		// INLINES
		inline RHI& bind_depth_stencil_target(RHI_DepthStencilView* depth_stencil)
		{
			return bind_render_target(nullptr, nullptr, nullptr, nullptr, depth_stencil);
		}

		inline RHI& bind_render_target1(RHI_RenderTargetView* rt1, RHI_DepthStencilView* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, nullptr, nullptr, nullptr, depth_stencil);
		}

		inline RHI& bind_render_target2(RHI_RenderTargetView* rt1, RHI_RenderTargetView* rt2,
		                                RHI_DepthStencilView* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, rt2, nullptr, nullptr, depth_stencil);
		}

		inline RHI& bind_render_target3(RHI_RenderTargetView* rt1, RHI_RenderTargetView* rt2, RHI_RenderTargetView* rt3,
		                                RHI_DepthStencilView* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, rt2, rt3, nullptr, depth_stencil);
		}

		inline RHI& bind_render_target4(RHI_RenderTargetView* rt1, RHI_RenderTargetView* rt2, RHI_RenderTargetView* rt3,
		                                RHI_RenderTargetView* rt4, RHI_DepthStencilView* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, rt2, rt3, rt4, depth_stencil);
		}

		inline RHI& update_scalar_parameter(const void* data, size_t size, const RHIShaderParameterInfo* info)
		{
			return update_scalar_parameter(data, size, info->offset, info->binding);
		}

		inline RHI& update_scalar_parameter(const void* data, const RHIShaderParameterInfo* info)
		{
			return update_scalar_parameter(data, info->size, info->offset, info->binding);
		}

		virtual ~RHI() {};
	};

	ENGINE_EXPORT extern RHI* rhi;

#if TRINEX_DEBUG_BUILD
#define trinex_rhi_push_stage Engine::rhi->push_debug_stage
#define trinex_rhi_pop_stage Engine::rhi->pop_debug_stage
#else
#define trinex_rhi_push_stage(...)
#define trinex_rhi_pop_stage(...)
#endif
}// namespace Engine
