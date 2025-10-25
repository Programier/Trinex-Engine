#pragma once
#include <Core/math/vector.hpp>
#include <RHI/object.hpp>
#include <RHI/structures.hpp>

namespace Engine
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
	private:
		struct State;
		State* m_state;

	public:
		RHIContext();
		~RHIContext();

		RHIContext& push_viewport(const RHIViewport& viewport);
		RHIContext& push_scissor(const RHIScissor& scissor);
		RHIContext& push_primitive_topology(RHIPrimitiveTopology topology);
		RHIContext& push_polygon_mode(RHIPolygonMode mode);
		RHIContext& push_cull_mode(RHICullMode mode);
		RHIContext& push_front_face(RHIFrontFace face);
		RHIContext& push_write_mask(RHIColorComponent mask);

		RHIContext& pop_viewport();
		RHIContext& pop_scissor();
		RHIContext& pop_primitive_topology();
		RHIContext& pop_polygon_mode();
		RHIContext& pop_cull_mode();
		RHIContext& pop_front_face();
		RHIContext& pop_write_mask();

	protected:
		RHIContext& reset_state();

	public:
		virtual RHIContext& begin()     = 0;
		virtual RHICommandHandle* end() = 0;

		virtual RHIContext& execute(RHICommandHandle* handle) = 0;

		virtual RHIContext& draw(size_t vertex_count, size_t vertices_offset, size_t instances = 1) = 0;
		virtual RHIContext& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset,
		                                 size_t instances = 1)                                      = 0;

		virtual RHIContext& draw_mesh(uint32_t x, uint32_t y, uint32_t z) = 0;

		virtual RHIContext& dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) = 0;

		virtual RHIContext& trace_rays(uint32_t width, uint32_t height, uint32_t depth, uint64_t raygen = 0,
		                               const RHIRange& miss = {}, const RHIRange& hit = {}, const RHIRange& callable = {}) = 0;

		virtual RHIContext& bind_render_target(RHIRenderTargetView* rt1, RHIRenderTargetView* rt2, RHIRenderTargetView* rt3,
		                                       RHIRenderTargetView* rt4, RHIDepthStencilView* depth_stencil) = 0;

		virtual RHIContext& viewport(const RHIViewport& viewport) = 0;
		virtual RHIContext& scissor(const RHIScissor& scissor)    = 0;

		virtual RHIContext& update_scalar(const void* data, size_t size, size_t offset, BindingIndex buffer_index) = 0;
		virtual RHIContext& push_debug_stage(const char* stage)                                                    = 0;
		virtual RHIContext& pop_debug_stage()                                                                      = 0;

		virtual RHIContext& clear_rtv(RHIRenderTargetView* rtv, float_t r = 0.f, float_t g = 0.f, float_t b = 0.f,
		                              float_t a = 0.f)                                                                       = 0;
		virtual RHIContext& clear_urtv(RHIRenderTargetView* rtv, uint_t r = 0u, uint_t g = 0u, uint_t b = 0u, uint_t a = 0u) = 0;
		virtual RHIContext& clear_irtv(RHIRenderTargetView* rtv, int_t r = 0, int_t g = 0, int_t b = 0, int_t a = 0)         = 0;

		virtual RHIContext& clear_dsv(RHIDepthStencilView* dsv, float_t depth = 1.f, byte stencil = 0) = 0;

		virtual RHIContext& update_buffer(RHIBuffer* buffer, size_t offset, size_t size, const byte* data) = 0;

		virtual RHIContext& update_texture(RHITexture* texture, const RHITextureRegion& region, const void* data, size_t size,
		                                   size_t buffer_width = 0, size_t buffer_height = 0) = 0;

		virtual RHIContext& copy_buffer_to_buffer(RHIBuffer* src, RHIBuffer* dst, size_t size, size_t src_offset,
		                                          size_t dst_offset) = 0;

		virtual RHIContext& copy_texture_to_buffer(RHITexture* texture, uint8_t mip_level, uint16_t array_slice,
		                                           const Vector3u& offset, const Vector3u& extent, RHIBuffer* buffer,
		                                           size_t buffer_offset) = 0;

		virtual RHIContext& copy_buffer_to_texture(RHIBuffer* buffer, size_t buffer_offset, RHITexture* texture,
		                                           uint8_t mip_level, uint16_t array_slice, const Vector3u& offset,
		                                           const Vector3u& extent) = 0;

		virtual RHIContext& copy_texture_to_texture(RHITexture* src, const RHITextureRegion& src_region, RHITexture* dst,
		                                            const RHITextureRegion& dst_region) = 0;

		virtual RHIContext& depth_state(const RHIDepthState& state)           = 0;
		virtual RHIContext& stencil_state(const RHIStencilState& state)       = 0;
		virtual RHIContext& blending_state(const RHIBlendingState& state)     = 0;
		virtual RHIContext& primitive_topology(RHIPrimitiveTopology topology) = 0;
		virtual RHIContext& polygon_mode(RHIPolygonMode mode)                 = 0;
		virtual RHIContext& cull_mode(RHICullMode mode)                       = 0;
		virtual RHIContext& front_face(RHIFrontFace face)                     = 0;
		virtual RHIContext& write_mask(RHIColorComponent mask)                = 0;

		virtual RHIContext& bind_vertex_attribute(RHIVertexSemantic semantic, RHIVertexFormat format, byte stream,
		                                          uint16_t offset = 0)                                          = 0;
		virtual RHIContext& bind_vertex_buffer(RHIBuffer* buffer, size_t byte_offset, uint16_t stride, byte stream,
		                                       RHIVertexInputRate rate = RHIVertexInputRate::Vertex)            = 0;
		virtual RHIContext& bind_index_buffer(RHIBuffer* buffer, RHIIndexFormat format, size_t byte_offset = 0) = 0;
		virtual RHIContext& bind_uniform_buffer(RHIBuffer* buffer, byte slot)                                   = 0;

		virtual RHIContext& bind_pipeline(RHIPipeline* pipeline)                                 = 0;
		virtual RHIContext& bind_sampler(RHISampler* sampler, byte slot)                         = 0;
		virtual RHIContext& bind_srv(RHIShaderResourceView* view, byte slot)                     = 0;
		virtual RHIContext& bind_uav(RHIUnorderedAccessView* view, byte slot)                    = 0;
		virtual RHIContext& bind_acceleration(RHIAccelerationStructure* acceleration, byte slot) = 0;

		virtual RHIContext& barrier(RHITexture* texture, RHIAccess access) = 0;
		virtual RHIContext& barrier(RHIBuffer* buffer, RHIAccess access)   = 0;

		virtual RHIContext& begin_timestamp(RHITimestamp* timestamp) = 0;
		virtual RHIContext& end_timestamp(RHITimestamp* timestamp)   = 0;

		virtual RHIContext& begin_statistics(RHIPipelineStatistics* stats) = 0;
		virtual RHIContext& end_statistics(RHIPipelineStatistics* stats)   = 0;

		inline RHIContext& bind_depth_stencil_target(RHIDepthStencilView* depth_stencil)
		{
			return bind_render_target(nullptr, nullptr, nullptr, nullptr, depth_stencil);
		}

		inline RHIContext& bind_render_target1(RHIRenderTargetView* rt1, RHIDepthStencilView* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, nullptr, nullptr, nullptr, depth_stencil);
		}

		inline RHIContext& bind_render_target2(RHIRenderTargetView* rt1, RHIRenderTargetView* rt2,
		                                       RHIDepthStencilView* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, rt2, nullptr, nullptr, depth_stencil);
		}

		inline RHIContext& bind_render_target3(RHIRenderTargetView* rt1, RHIRenderTargetView* rt2, RHIRenderTargetView* rt3,
		                                       RHIDepthStencilView* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, rt2, rt3, nullptr, depth_stencil);
		}

		inline RHIContext& bind_render_target4(RHIRenderTargetView* rt1, RHIRenderTargetView* rt2, RHIRenderTargetView* rt3,
		                                       RHIRenderTargetView* rt4, RHIDepthStencilView* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, rt2, rt3, rt4, depth_stencil);
		}

		inline RHIContext& update_scalar(const void* data, size_t size, const RHIShaderParameterInfo* info)
		{
			return update_scalar(data, size, info->offset, info->binding);
		}

		inline RHIContext& update_scalar(const void* data, const RHIShaderParameterInfo* info)
		{
			return update_scalar(data, info->size, info->offset, info->binding);
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
}// namespace Engine
