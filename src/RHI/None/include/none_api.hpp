#pragma once

#include <Graphics/rhi.hpp>


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

		NoneApi& dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) override;

		NoneApi& submit() override;

		NoneApi& bind_render_target(RHI_RenderTargetView* rt1, RHI_RenderTargetView* rt2, RHI_RenderTargetView* rt3,
		                            RHI_RenderTargetView* rt4, RHI_DepthStencilView* depth_stencil) override;
		NoneApi& viewport(const ViewPort& viewport) override;
		ViewPort viewport() override;
		NoneApi& scissor(const Scissor& scissor) override;
		Scissor scissor() override;
		RHI_Sampler* create_sampler(const Sampler*) override;
		RHI_Texture2D* create_texture_2d(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags) override;
		RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
		RHI_Shader* create_tesselation_control_shader(const TessellationControlShader* shader) override;
		RHI_Shader* create_tesselation_shader(const TessellationShader* shader) override;
		RHI_Shader* create_geometry_shader(const GeometryShader* shader) override;
		RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
		RHI_Shader* create_compute_shader(const ComputeShader* shader) override;
		RHI_Pipeline* create_graphics_pipeline(const GraphicsPipeline* pipeline) override;
		RHI_Pipeline* create_compute_pipeline(const ComputePipeline* pipeline) override;
		RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data, RHIBufferType type) override;
		RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, RHIIndexFormat format, RHIBufferType type) override;
		RHI_SSBO* create_ssbo(size_t size, const byte* data, RHIBufferType type) override;
		RHI_UniformBuffer* create_uniform_buffer(size_t size, const byte* data, RHIBufferType type) override;
		RHI_Viewport* create_viewport(WindowRenderViewport* viewport, bool vsync) override;

		NoneApi& update_scalar_parameter(const void* data, size_t size, size_t offset, BindingIndex buffer_index) override;
		NoneApi& push_debug_stage(const char* stage, const LinearColor& color = {}) override;
		NoneApi& pop_debug_stage() override;
	};
}// namespace Engine
