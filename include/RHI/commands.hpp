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
	class RHIPipelineStatistics;

	class ENGINE_EXPORT RHICommandHandle : public RHIObject
	{
	};

	class ENGINE_EXPORT RHICommandBuffer : public RHIObject
	{
	public:
		virtual RHICommandBuffer& begin() = 0;
		virtual RHICommandHandle* end()   = 0;

		virtual RHICommandBuffer& execute(RHICommandHandle* handle) = 0;

		virtual RHICommandBuffer& draw(size_t vertex_count, size_t vertices_offset)                                 = 0;
		virtual RHICommandBuffer& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) = 0;
		virtual RHICommandBuffer& draw_instanced(size_t vertex_count, size_t vertex_offset, size_t instances)       = 0;
		virtual RHICommandBuffer& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
		                                                 size_t instances)                                          = 0;

		virtual RHICommandBuffer& draw_mesh(uint32_t x, uint32_t y, uint32_t z) = 0;

		virtual RHICommandBuffer& dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) = 0;
		virtual RHICommandBuffer& signal_fence(RHIFence* fence)                                  = 0;

		virtual RHICommandBuffer& bind_render_target(RHIRenderTargetView* rt1, RHIRenderTargetView* rt2, RHIRenderTargetView* rt3,
		                                             RHIRenderTargetView* rt4, RHIDepthStencilView* depth_stencil) = 0;

		virtual RHICommandBuffer& viewport(const RHIViewport& viewport) = 0;
		virtual RHICommandBuffer& scissor(const RHIScissors& scissor)   = 0;

		virtual RHICommandBuffer& update_scalar(const void* data, size_t size, size_t offset, BindingIndex buffer_index) = 0;
		virtual RHICommandBuffer& push_debug_stage(const char* stage)                                                    = 0;
		virtual RHICommandBuffer& pop_debug_stage()                                                                      = 0;

		virtual RHICommandBuffer& update_buffer(RHIBuffer* buffer, size_t offset, size_t size, const byte* data) = 0;

		virtual RHICommandBuffer& update_texture(RHITexture* texture, const RHITextureRegion& region, const void* data,
		                                         size_t size, size_t buffer_width = 0, size_t buffer_height = 0) = 0;

		virtual RHICommandBuffer& copy_buffer_to_buffer(RHIBuffer* src, RHIBuffer* dst, size_t size, size_t src_offset,
		                                                size_t dst_offset) = 0;

		virtual RHICommandBuffer& copy_texture_to_buffer(RHITexture* texture, uint8_t mip_level, uint16_t array_slice,
		                                                 const Vector3u& offset, const Vector3u& extent, RHIBuffer* buffer,
		                                                 size_t buffer_offset) = 0;

		virtual RHICommandBuffer& copy_buffer_to_texture(RHIBuffer* buffer, size_t buffer_offset, RHITexture* texture,
		                                                 uint8_t mip_level, uint16_t array_slice, const Vector3u& offset,
		                                                 const Vector3u& extent) = 0;

		virtual RHICommandBuffer& copy_texture_to_texture(RHITexture* src, const RHITextureRegion& src_region, RHITexture* dst,
		                                                  const RHITextureRegion& dst_region) = 0;

		virtual RHICommandBuffer& primitive_topology(RHIPrimitiveTopology topology) = 0;
		virtual RHICommandBuffer& polygon_mode(RHIPolygonMode mode)                 = 0;
		virtual RHICommandBuffer& cull_mode(RHICullMode mode)                       = 0;
		virtual RHICommandBuffer& front_face(RHIFrontFace face)                     = 0;
		virtual RHICommandBuffer& write_mask(RHIColorComponent mask)                = 0;

		virtual RHICommandBuffer& bind_vertex_attribute(RHIVertexSemantic semantic, byte semantic_index, byte stream,
		                                                uint16_t offset = 0)                               = 0;
		virtual RHICommandBuffer& bind_vertex_buffer(RHIBuffer* buffer, size_t byte_offset, uint16_t stride, byte stream,
		                                             RHIVertexInputRate rate = RHIVertexInputRate::Vertex) = 0;
		virtual RHICommandBuffer& bind_index_buffer(RHIBuffer* buffer, RHIIndexFormat format)              = 0;
		virtual RHICommandBuffer& bind_uniform_buffer(RHIBuffer* buffer, byte slot)                        = 0;

		virtual RHICommandBuffer& bind_sampler(RHISampler* sampler, byte slot)      = 0;
		virtual RHICommandBuffer& bind_srv(RHIShaderResourceView* view, byte slot)  = 0;
		virtual RHICommandBuffer& bind_uav(RHIUnorderedAccessView* view, byte slot) = 0;

		virtual RHICommandBuffer& barrier(RHITexture* texture, RHIAccess access) = 0;
		virtual RHICommandBuffer& barrier(RHIBuffer* buffer, RHIAccess access)   = 0;

		virtual RHICommandBuffer& begin_timestamp(RHITimestamp* timestamp) = 0;
		virtual RHICommandBuffer& end_timestamp(RHITimestamp* timestamp)   = 0;

		virtual RHICommandBuffer& begin_statistics(RHIPipelineStatistics* stats) = 0;
		virtual RHICommandBuffer& end_statistics(RHIPipelineStatistics* stats)   = 0;
	};
}// namespace Engine
