
#pragma once
#include <Core/etl/vector.hpp>
#include <Core/logger.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader_parameters.hpp>
#include <opengl_definitions.hpp>
#include <opengl_headers.hpp>

#define OPENGL_API Engine::OpenGL::m_instance

namespace Engine
{
	struct OpenGL_RenderSurface;

	struct OpenGL_State {
		ViewPort viewport                         = {};
		Scissor scissor                           = {};
		struct OpenGL_RenderTarget* render_target = nullptr;
		struct OpenGL_Pipeline* pipeline          = nullptr;
		struct OpenGL_IndexBuffer* index_buffer   = nullptr;
	};

	struct OpenGL : public RHI {
		trinex_declare_struct(OpenGL, void);
		static OpenGL* static_constructor();
		static void static_destructor(OpenGL* opengl);

		static OpenGL* m_instance;
		void* m_context = nullptr;

		Vector<BindingIndex> m_sampler_units;// TODO: Maybe we can remove this variable?
		GLint m_uniform_alignment                            = 0;
		struct OpenGL_LocalUniformBufferManager* m_local_ubo = nullptr;

		OpenGL_State m_state;

		OpenGL();
		OpenGL& initialize(Window* window) override;
		OpenGL& initialize_ubo();
		void* context() override;

		OpenGL& on_draw();
		OpenGL& on_dispatch();
		OpenGL& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) override;
		OpenGL& draw(size_t vertex_count, size_t vertices_offset) override;
		OpenGL& draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances) override;
		OpenGL& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
		                               size_t instances) override;
		OpenGL& dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) override;
		OpenGL& submit() override;
		OpenGL& reset_state();

		OpenGL& bind_render_target(RHI_RenderTargetView* rt1,//
		                           RHI_RenderTargetView* rt2,//
		                           RHI_RenderTargetView* rt3,//
		                           RHI_RenderTargetView* rt4,//
		                           RHI_DepthStencilView* depth_stencil) override;

		OpenGL& viewport(const ViewPort& viewport) override;
		ViewPort viewport() override;
		OpenGL& scissor(const Scissor& scissor) override;
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
		OpenGL& update_scalar_parameter(const void* data, size_t size, size_t offset, BindingIndex buffer_index) override;

		OpenGL& push_debug_stage(const char* stage, const LinearColor& color = {}) override;
		OpenGL& pop_debug_stage() override;

		void reset_samplers();

		~OpenGL();
	};
}// namespace Engine
