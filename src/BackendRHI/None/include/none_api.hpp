#pragma once

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

		NoneApi& initialize(class Window* window) override;
		void* context() override;

		NoneApi& draw(size_t vertex_count, size_t vertices_offset) override;
		NoneApi& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) override;
		NoneApi& draw_instanced(size_t vertex_count, size_t vertex_offset, size_t instances) override;
		NoneApi& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
		                                size_t instances) override;

		NoneApi& draw_mesh(uint32_t x, uint32_t y, uint32_t z) override;

		NoneApi& dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) override;
		NoneApi& signal_fence(RHI_Fence* fence) override;
		NoneApi& submit() override;

		NoneApi& bind_render_target(RHI_RenderTargetView* rt1, RHI_RenderTargetView* rt2, RHI_RenderTargetView* rt3,
		                            RHI_RenderTargetView* rt4, RHI_DepthStencilView* depth_stencil) override;
		NoneApi& viewport(const RHIViewport& viewport) override;
		NoneApi& scissor(const RHIScissors& scissor) override;

		RHITimestamp* create_timestamp() override;
		RHIPipelineStatistics* create_pipeline_statistics() override;
		RHI_Fence* create_fence() override;
		RHI_Sampler* create_sampler(const RHISamplerInitializer*) override;
		RHI_Texture* create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, uint32_t mips,
		                            RHITextureCreateFlags flags) override;
		RHI_Shader* create_shader(const byte* source, size_t size) override;
		RHI_Pipeline* create_graphics_pipeline(const RHIGraphicsPipelineInitializer* pipeline) override;
		RHI_Pipeline* create_mesh_pipeline(const RHIMeshPipelineInitializer* pipeline) override;
		RHI_Pipeline* create_compute_pipeline(const RHIComputePipelineInitializer* pipeline) override;
		RHI_Buffer* create_buffer(size_t size, const byte* data, RHIBufferCreateFlags type) override;
		RHI_Viewport* create_viewport(WindowRenderViewport* viewport, bool vsync) override;

		NoneApi& primitive_topology(RHIPrimitiveTopology topology) override;
		NoneApi& polygon_mode(RHIPolygonMode mode) override;
		NoneApi& cull_mode(RHICullMode mode) override;
		NoneApi& front_face(RHIFrontFace face) override;

		NoneApi& bind_vertex_buffer(RHI_Buffer* buffer, size_t byte_offset, uint16_t stride, byte stream) override;
		NoneApi& bind_index_buffer(RHI_Buffer* buffer, RHIIndexFormat format) override;
		NoneApi& bind_uniform_buffer(RHI_Buffer* buffer, byte slot) override;
		NoneApi& bind_srv(RHI_ShaderResourceView* view, byte slot) override;
		NoneApi& bind_uav(RHI_UnorderedAccessView* view, byte slot) override;
		NoneApi& bind_sampler(RHI_Sampler* sampler, byte slot) override;

		NoneApi& update_buffer(RHI_Buffer* buffer, size_t offset, size_t size, const byte* data) override;
		NoneApi& update_texture(RHI_Texture*, const RHITextureUpdateDesc& desc) override;

		NoneApi& copy_buffer_to_buffer(RHI_Buffer* src, RHI_Buffer* dst, size_t size, size_t src_offset,
		                               size_t dst_offset) override;

		NoneApi& copy_texture_to_buffer(RHI_Texture* texture, uint8_t mip_level, uint16_t array_slice, const Vector3u& offset,
		                                const Vector3u& extent, RHI_Buffer* buffer, size_t buffer_offset) override;

		NoneApi& copy_buffer_to_texture(RHI_Buffer* buffer, size_t buffer_offset, RHI_Texture* texture, uint8_t mip_level,
		                                uint16_t array_slice, const Vector3u& offset, const Vector3u& extent) override;

		NoneApi& barrier(RHI_Texture* texture, RHIAccess dst_access) override;
		NoneApi& barrier(RHI_Buffer* buffer, RHIAccess dst_access) override;

		NoneApi& begin_timestamp(RHITimestamp* timestamp) override;
		NoneApi& end_timestamp(RHITimestamp* timestamp) override;

		NoneApi& begin_statistics(RHIPipelineStatistics* stats) override;
		NoneApi& end_statistics(RHIPipelineStatistics* stats) override;

		NoneApi& update_scalar_parameter(const void* data, size_t size, size_t offset, BindingIndex buffer_index) override;
		NoneApi& push_debug_stage(const char* stage) override;
		NoneApi& pop_debug_stage() override;
	};
}// namespace Engine
