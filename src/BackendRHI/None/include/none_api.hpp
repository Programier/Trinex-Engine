#pragma once

#include <RHI/handles.hpp>
#include <RHI/rhi.hpp>


namespace Engine
{
	class NoneApi : public RHI
	{
	public:
		trinex_declare_struct(NoneApi, void);
		static NoneApi* static_constructor();
		static void static_destructor(NoneApi* api);

		static NoneApi* m_instance;

		NoneApi& draw(size_t vertex_count, size_t vertices_offset) override;
		NoneApi& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) override;
		NoneApi& draw_instanced(size_t vertex_count, size_t vertex_offset, size_t instances) override;
		NoneApi& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
		                                size_t instances) override;

		NoneApi& draw_mesh(uint32_t x, uint32_t y, uint32_t z) override;

		NoneApi& dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) override;
		NoneApi& trace_rays(uint32_t width, uint32_t height, uint32_t depth, uint64_t raygen = 0, const RHIRange& miss = {},
		                    const RHIRange& hit = {}, const RHIRange& callable = {}) override;

		NoneApi& signal_fence(RHIFence* fence) override;
		NoneApi& submit() override;

		NoneApi& bind_render_target(RHIRenderTargetView* rt1, RHIRenderTargetView* rt2, RHIRenderTargetView* rt3,
		                            RHIRenderTargetView* rt4, RHIDepthStencilView* depth_stencil) override;
		NoneApi& viewport(const RHIViewport& viewport) override;
		NoneApi& scissor(const RHIScissors& scissor) override;

		RHITimestamp* create_timestamp() override;
		RHIPipelineStatistics* create_pipeline_statistics() override;
		RHIFence* create_fence() override;
		RHISampler* create_sampler(const RHISamplerInitializer*) override;
		RHITexture* create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, uint32_t mips,
		                           RHITextureCreateFlags flags) override;
		RHIShader* create_shader(const byte* source, size_t size) override;
		RHIPipeline* create_graphics_pipeline(const RHIGraphicsPipelineInitializer* pipeline) override;
		RHIPipeline* create_mesh_pipeline(const RHIMeshPipelineInitializer* pipeline) override;
		RHIPipeline* create_compute_pipeline(const RHIComputePipelineInitializer* pipeline) override;
		RHIPipeline* create_ray_tracing_pipeline(const RHIRayTracingPipelineInitializer* pipeline) override;
		RHIBuffer* create_buffer(size_t size, const byte* data, RHIBufferCreateFlags type) override;
		RHISwapchain* create_swapchain(Window* window, bool vsync) override;
		RHIContext* create_context() override;

		RHIAccelerationStructure* create_acceleration_structure(const RHIRayTracingAccelerationInputs* inputs) override;
		const byte* translate_ray_tracing_instances(const RHIRayTracingGeometryInstance* instances, size_t& size) override;

		NoneApi& primitive_topology(RHIPrimitiveTopology topology) override;
		NoneApi& polygon_mode(RHIPolygonMode mode) override;
		NoneApi& cull_mode(RHICullMode mode) override;
		NoneApi& front_face(RHIFrontFace face) override;
		NoneApi& write_mask(RHIColorComponent mask) override;

		NoneApi& bind_vertex_attribute(RHIVertexSemantic semantic, byte semantic_index, byte stream, uint16_t offset) override;
		NoneApi& bind_vertex_buffer(RHIBuffer* buffer, size_t byte_offset, uint16_t stride, byte stream,
		                            RHIVertexInputRate rate) override;
		NoneApi& bind_index_buffer(RHIBuffer* buffer, RHIIndexFormat format) override;
		NoneApi& bind_uniform_buffer(RHIBuffer* buffer, byte slot) override;
		NoneApi& bind_srv(RHIShaderResourceView* view, byte slot) override;
		NoneApi& bind_uav(RHIUnorderedAccessView* view, byte slot) override;
		NoneApi& bind_sampler(RHISampler* sampler, byte slot) override;
		NoneApi& bind_acceleration(RHIAccelerationStructure* acceleration, byte slot) override;

		NoneApi& update_buffer(RHIBuffer* buffer, size_t offset, size_t size, const byte* data) override;
		NoneApi& update_texture(RHITexture* texture, const RHITextureRegion& region, const void* data, size_t size,
		                        size_t buffer_width = 0, size_t buffer_height = 0) override;

		NoneApi& copy_buffer_to_buffer(RHIBuffer* src, RHIBuffer* dst, size_t size, size_t src_offset,
		                               size_t dst_offset) override;

		NoneApi& copy_texture_to_buffer(RHITexture* texture, uint8_t mip_level, uint16_t array_slice, const Vector3u& offset,
		                                const Vector3u& extent, RHIBuffer* buffer, size_t buffer_offset) override;

		NoneApi& copy_buffer_to_texture(RHIBuffer* buffer, size_t buffer_offset, RHITexture* texture, uint8_t mip_level,
		                                uint16_t array_slice, const Vector3u& offset, const Vector3u& extent) override;

		NoneApi& copy_texture_to_texture(RHITexture* src, const RHITextureRegion& src_region, RHITexture* dst,
		                                 const RHITextureRegion& dst_region) override;

		NoneApi& barrier(RHITexture* texture, RHIAccess dst_access) override;
		NoneApi& barrier(RHIBuffer* buffer, RHIAccess dst_access) override;

		NoneApi& begin_timestamp(RHITimestamp* timestamp) override;
		NoneApi& end_timestamp(RHITimestamp* timestamp) override;

		NoneApi& begin_statistics(RHIPipelineStatistics* stats) override;
		NoneApi& end_statistics(RHIPipelineStatistics* stats) override;

		NoneApi& present(RHISwapchain* swapchain) override;

		NoneApi& update_scalar(const void* data, size_t size, size_t offset, BindingIndex buffer_index) override;
		NoneApi& push_debug_stage(const char* stage) override;
		NoneApi& pop_debug_stage() override;
	};
}// namespace Engine
