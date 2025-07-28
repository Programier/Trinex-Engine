#pragma once
#include <Core/types/color.hpp>
#include <RHI/enums.hpp>
#include <RHI/handles.hpp>
#include <RHI/structures.hpp>

namespace Engine
{
	class Window;
	class GraphicsPipeline;
	class ComputePipeline;

	struct RHISamplerInitializer;
	struct RHIGraphicsPipelineInitializer;
	struct RHIMeshPipelineInitializer;
	struct RHIComputePipelineInitializer;

	struct LinearColor;

	namespace Refl
	{
		class Struct;
	}

	class ENGINE_EXPORT RHI
	{
	public:
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

		virtual RHI& draw(size_t vertex_count, size_t vertices_offset)                                 = 0;
		virtual RHI& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) = 0;
		virtual RHI& draw_instanced(size_t vertex_count, size_t vertex_offset, size_t instances)       = 0;
		virtual RHI& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
		                                    size_t instances)                                          = 0;

		virtual RHI& draw_mesh(uint32_t x, uint32_t y, uint32_t z) = 0;

		virtual RHI& dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) = 0;
		virtual RHI& signal_fence(RHIFence* fence)                                  = 0;
		virtual RHI& submit()                                                       = 0;

		virtual RHI& bind_render_target(RHIRenderTargetView* rt1, RHIRenderTargetView* rt2, RHIRenderTargetView* rt3,
		                                RHIRenderTargetView* rt4, RHIDepthStencilView* depth_stencil) = 0;

		virtual RHI& viewport(const RHIViewport& viewport) = 0;
		virtual RHI& scissor(const RHIScissors& scissor)   = 0;

		virtual RHITimestamp* create_timestamp()                                                                      = 0;
		virtual RHIPipelineStatistics* create_pipeline_statistics()                                                   = 0;
		virtual RHIFence* create_fence()                                                                              = 0;
		virtual RHISampler* create_sampler(const RHISamplerInitializer*)                                              = 0;
		virtual RHITexture* create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, uint32_t mips,
		                                   RHITextureCreateFlags flags)                                               = 0;
		virtual RHIShader* create_shader(const byte* shader, size_t size)                                             = 0;
		virtual RHIPipeline* create_graphics_pipeline(const RHIGraphicsPipelineInitializer* pipeline)                 = 0;
		virtual RHIPipeline* create_mesh_pipeline(const RHIMeshPipelineInitializer* pipeline)                         = 0;
		virtual RHIPipeline* create_compute_pipeline(const RHIComputePipelineInitializer* pipeline)                   = 0;
		virtual RHIBuffer* create_buffer(size_t size, const byte* data, RHIBufferCreateFlags flags)                   = 0;
		virtual RHISwapchain* create_swapchain(Window* window, bool vsync)                                            = 0;
		virtual RHI& update_scalar_parameter(const void* data, size_t size, size_t offset, BindingIndex buffer_index) = 0;
		virtual RHI& push_debug_stage(const char* stage)                                                              = 0;
		virtual RHI& pop_debug_stage()                                                                                = 0;

		virtual RHI& update_buffer(RHIBuffer* buffer, size_t offset, size_t size, const byte* data) = 0;

		virtual RHI& update_texture(RHITexture* texture, const RHITextureRegion& region, const void* data, size_t size,
		                            size_t buffer_width = 0, size_t buffer_height = 0) = 0;

		virtual RHI& copy_buffer_to_buffer(RHIBuffer* src, RHIBuffer* dst, size_t size, size_t src_offset, size_t dst_offset) = 0;

		virtual RHI& copy_texture_to_buffer(RHITexture* texture, uint8_t mip_level, uint16_t array_slice, const Vector3u& offset,
		                                    const Vector3u& extent, RHIBuffer* buffer, size_t buffer_offset) = 0;

		virtual RHI& copy_buffer_to_texture(RHIBuffer* buffer, size_t buffer_offset, RHITexture* texture, uint8_t mip_level,
		                                    uint16_t array_slice, const Vector3u& offset, const Vector3u& extent) = 0;

		virtual RHI& copy_texture_to_texture(RHITexture* src, const RHITextureRegion& src_region, RHITexture* dst,
		                                     const RHITextureRegion& dst_region) = 0;

		virtual RHI& primitive_topology(RHIPrimitiveTopology topology) = 0;
		virtual RHI& polygon_mode(RHIPolygonMode mode)                 = 0;
		virtual RHI& cull_mode(RHICullMode mode)                       = 0;
		virtual RHI& front_face(RHIFrontFace face)                     = 0;
		virtual RHI& write_mask(RHIColorComponent mask)                = 0;

		virtual RHI& bind_vertex_buffer(RHIBuffer* buffer, size_t byte_offset, uint16_t stride, byte stream) = 0;
		virtual RHI& bind_index_buffer(RHIBuffer* buffer, RHIIndexFormat format)                             = 0;
		virtual RHI& bind_uniform_buffer(RHIBuffer* buffer, byte slot)                                       = 0;

		virtual RHI& bind_sampler(RHISampler* sampler, byte slot)      = 0;
		virtual RHI& bind_srv(RHIShaderResourceView* view, byte slot)  = 0;
		virtual RHI& bind_uav(RHIUnorderedAccessView* view, byte slot) = 0;

		virtual RHI& barrier(RHITexture* texture, RHIAccess access) = 0;
		virtual RHI& barrier(RHIBuffer* buffer, RHIAccess access)   = 0;

		virtual RHI& begin_timestamp(RHITimestamp* timestamp) = 0;
		virtual RHI& end_timestamp(RHITimestamp* timestamp)   = 0;

		virtual RHI& begin_statistics(RHIPipelineStatistics* stats) = 0;
		virtual RHI& end_statistics(RHIPipelineStatistics* stats)   = 0;

		virtual RHI& present(RHISwapchain* swapchain) = 0;

		// INLINES
		inline RHI& bind_depth_stencil_target(RHIDepthStencilView* depth_stencil)
		{
			return bind_render_target(nullptr, nullptr, nullptr, nullptr, depth_stencil);
		}

		inline RHI& bind_render_target1(RHIRenderTargetView* rt1, RHIDepthStencilView* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, nullptr, nullptr, nullptr, depth_stencil);
		}

		inline RHI& bind_render_target2(RHIRenderTargetView* rt1, RHIRenderTargetView* rt2,
		                                RHIDepthStencilView* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, rt2, nullptr, nullptr, depth_stencil);
		}

		inline RHI& bind_render_target3(RHIRenderTargetView* rt1, RHIRenderTargetView* rt2, RHIRenderTargetView* rt3,
		                                RHIDepthStencilView* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, rt2, rt3, nullptr, depth_stencil);
		}

		inline RHI& bind_render_target4(RHIRenderTargetView* rt1, RHIRenderTargetView* rt2, RHIRenderTargetView* rt3,
		                                RHIRenderTargetView* rt4, RHIDepthStencilView* depth_stencil = nullptr)
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
