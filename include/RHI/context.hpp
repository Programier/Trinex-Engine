#pragma once
#include <Core/math/vector.hpp>
#include <RHI/object.hpp>
#include <RHI/structures.hpp>

namespace Trinex
{
	class RHIBuffer;
	class RHITexture;
	class RHISampler;
	class RHIFence;
	class RHIRenderTargetView;
	class RHIDepthStencilView;
	class RHIShaderResourceView;
	class RHIUnorderedAccessView;
	class RHITimestamp;
	class RHIPipeline;
	class RHIPipelineStatistics;

	class ENGINE_EXPORT RHICommandHandle : public RHIObject
	{
	};

	class ENGINE_EXPORT RHIContext : public RHIObject
	{
	public:
		RHIContext();
		~RHIContext();

	public:
		virtual RHIContext& begin(const RHIContextInheritanceInfo* inheritance = nullptr) = 0;
		virtual RHICommandHandle* end()                                                   = 0;

		virtual RHIContext& begin_rendering(const RHIRenderingInfo& info) = 0;
		virtual RHIContext& end_rendering()                               = 0;

		virtual RHIContext& execute(RHICommandHandle* handle) = 0;

		virtual RHIContext& draw(RHITopology topology, usize vertex_count, usize vertices_offset, usize instances = 1) = 0;
		virtual RHIContext& draw_indexed(RHITopology topology, usize indices_count, usize indices_offset, usize vertices_offset,
		                                 usize instances = 1)                                                          = 0;

		virtual RHIContext& draw_mesh(u32 x, u32 y, u32 z) = 0;

		virtual RHIContext& dispatch(u32 group_x, u32 group_y, u32 group_z) = 0;

		virtual RHIContext& trace_rays(u32 width, u32 height, u32 depth, u64 raygen = 0, const RHIRange& miss = {},
		                               const RHIRange& hit = {}, const RHIRange& callable = {}) = 0;

		virtual RHIContext& viewport(const RHIViewport& viewport) = 0;
		virtual RHIContext& scissor(const RHIScissor& scissor)    = 0;

		virtual RHIContext& update_scalar(const void* data, usize size, usize offset, u8 buffer_index) = 0;
		virtual RHIContext& push_debug_stage(const char* stage)                                        = 0;
		virtual RHIContext& pop_debug_stage()                                                          = 0;

		virtual RHIContext& clear_rtv(RHIRenderTargetView* rtv, f32 r = 0.f, f32 g = 0.f, f32 b = 0.f, f32 a = 0.f) = 0;
		virtual RHIContext& clear_urtv(RHIRenderTargetView* rtv, u32 r = 0u, u32 g = 0u, u32 b = 0u, u32 a = 0u)    = 0;
		virtual RHIContext& clear_irtv(RHIRenderTargetView* rtv, i32 r = 0, i32 g = 0, i32 b = 0, i32 a = 0)        = 0;

		virtual RHIContext& clear_dsv(RHIDepthStencilView* dsv, f32 depth = 0.f, u8 stencil = 0) = 0;

		virtual RHIContext& update_buffer(RHIBuffer* buffer, usize offset, usize size, const u8* data) = 0;

		virtual RHIContext& update_texture(RHITexture* texture, const RHITextureRegion& region, const void* data, usize size,
		                                   usize buffer_width = 0, usize buffer_height = 0) = 0;

		virtual RHIContext& copy_buffer_to_buffer(RHIBuffer* src, RHIBuffer* dst, usize size, usize src_offset,
		                                          usize dst_offset) = 0;

		virtual RHIContext& copy_texture_to_buffer(RHITexture* texture, u8 mip_level, u16 array_slice, const Vector3u& offset,
		                                           const Vector3u& extent, RHIBuffer* buffer, usize buffer_offset) = 0;

		virtual RHIContext& copy_buffer_to_texture(RHIBuffer* buffer, usize buffer_offset, RHITexture* texture, u8 mip_level,
		                                           u16 array_slice, const Vector3u& offset, const Vector3u& extent) = 0;

		virtual RHIContext& copy_texture_to_texture(RHITexture* src, const RHITextureRegion& src_region, RHITexture* dst,
		                                            const RHITextureRegion& dst_region) = 0;

		virtual RHIContext& depth_stencil_state(const RHIDepthStencilState& state) = 0;
		virtual RHIContext& blending_state(const RHIBlendingState& state)          = 0;
		virtual RHIContext& rasterizer_state(const RHIRasterizerState& state)      = 0;

		virtual RHIContext& depth_bias(float constant = 0.0f, float clamp = 0.0f, float slope = 0.0f) = 0;

		virtual RHIContext& bind_vertex_attribute(RHIVertexSemantic semantic, RHIVertexFormat format, u8 stream,
		                                          u16 offset = 0)                                              = 0;
		virtual RHIContext& bind_vertex_buffer(RHIBuffer* buffer, usize byte_offset, u16 stride, u8 stream,
		                                       RHIVertexInputRate rate = RHIVertexInputRate::Vertex)           = 0;
		virtual RHIContext& bind_index_buffer(RHIBuffer* buffer, RHIIndexFormat format, usize byte_offset = 0) = 0;
		virtual RHIContext& bind_uniform_buffer(RHIBuffer* buffer, u8 slot)                                    = 0;

		virtual RHIContext& bind_pipeline(RHIPipeline* pipeline)                               = 0;
		virtual RHIContext& bind_sampler(RHISampler* sampler, u8 slot)                         = 0;
		virtual RHIContext& bind_srv(RHIShaderResourceView* view, u8 slot)                     = 0;
		virtual RHIContext& bind_uav(RHIUnorderedAccessView* view, u8 slot)                    = 0;
		virtual RHIContext& bind_acceleration(RHIAccelerationStructure* acceleration, u8 slot) = 0;

		virtual RHIContext& barrier(RHITexture* texture, RHIAccess access) = 0;
		virtual RHIContext& barrier(RHIBuffer* buffer, RHIAccess access)   = 0;

		virtual RHIContext& begin_timestamp(RHITimestamp* timestamp) = 0;
		virtual RHIContext& end_timestamp(RHITimestamp* timestamp)   = 0;

		virtual RHIContext& begin_statistics(RHIPipelineStatistics* stats) = 0;
		virtual RHIContext& end_statistics(RHIPipelineStatistics* stats)   = 0;

		inline RHIContext& update_scalar(const void* data, usize size, const RHIShaderParameterInfo* info)
		{
			return update_scalar(data, size, info->offset, info->binding);
		}

		inline RHIContext& update_scalar(const void* data, const RHIShaderParameterInfo* info)
		{
			return update_scalar(data, info->size, info->offset, info->binding);
		}

		template<typename Func>
		inline RHIContext& render(const RHIRenderingInfo& info, Func&& func)
		{
			begin_rendering(info);
			func();
			return end_rendering();
		}
	};

#if TRINEX_DEBUG_BUILD
#define trinex_rhi_push_stage(ctx, name) ctx->push_debug_stage(name)
#define trinex_rhi_pop_stage(ctx) ctx->pop_debug_stage()
#else
#define trinex_rhi_push_stage(ctx, name)                                                                                         \
	do                                                                                                                           \
	{                                                                                                                            \
	} while (false)
#define trinex_rhi_pop_stage(ctx)                                                                                                \
	do                                                                                                                           \
	{                                                                                                                            \
	} while (false)
#endif
}// namespace Trinex
