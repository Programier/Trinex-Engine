#pragma once
#include <Core/logger.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader_parameters.hpp>
#include <opengl_definitions.hpp>
#include <opengl_headers.hpp>

#define OPENGL_API Engine::OpenGL::m_instance

namespace Engine
{
	struct OpenGL_State {
		ViewPort viewport                         = {};
		Scissor scissor                           = {};
		struct OpenGL_RenderTarget* render_target = nullptr;
		struct OpenGL_Pipeline* pipeline          = nullptr;
		struct OpenGL_IndexBuffer* index_buffer   = nullptr;
	};

	struct OpenGL : public RHI {
		declare_struct(OpenGL, void);
		static OpenGL* static_constructor();
		static void static_destructor(OpenGL* opengl);
		
		static OpenGL* m_instance;
		void* m_context = nullptr;

		Vector<BindingIndex> m_sampler_units;// TODO: Maybe we can remove this variable?
		Vector<GlobalShaderParameters> m_global_parameters_stack;
		struct OpenGL_UniformBuffer* m_global_ubo     = nullptr;
		struct OpenGL_LocalUniformBuffer* m_local_ubo = nullptr;

		OpenGL_State m_state;

		OpenGL();
		OpenGL& initialize(Window* window) override;
		OpenGL& initialize_ubo();
		void* context() override;

		OpenGL& prepare_render();
		OpenGL& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) override;
		OpenGL& draw(size_t vertex_count, size_t vertices_offset) override;
		OpenGL& draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances) override;
		OpenGL& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
		                               size_t instances) override;
		OpenGL& begin_render() override;
		OpenGL& end_render() override;
		OpenGL& reset_state();

		OpenGL& bind_render_target(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil) override;
		OpenGL& bind_render_target(const Span<struct OpenGL_RenderSurface*>& color_attachments,
		                           struct OpenGL_RenderSurface* depth_stencil);
		OpenGL& viewport(const ViewPort& viewport) override;
		ViewPort viewport() override;
		OpenGL& scissor(const Scissor& scissor) override;
		Scissor scissor() override;

		RHI_Sampler* create_sampler(const Sampler*) override;
		RHI_Texture* create_texture_2d(const Texture2D*) override;
		RHI_Texture* create_render_surface(const RenderSurface*) override;
		RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
		RHI_Shader* create_tesselation_control_shader(const TessellationControlShader* shader) override;
		RHI_Shader* create_tesselation_shader(const TessellationShader* shader) override;
		RHI_Shader* create_geometry_shader(const GeometryShader* shader) override;
		RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
		RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;
		RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data, RHIBufferType type) override;
		RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, IndexBufferFormat format, RHIBufferType type) override;
		RHI_SSBO* create_ssbo(size_t size, const byte* data, RHIBufferType type) override;
		RHI_Viewport* create_viewport(RenderViewport* viewport) override;

		OpenGL& push_global_params(const GlobalShaderParameters& params) override;
		OpenGL& pop_global_params() override;
		OpenGL& update_local_parameter(const void* data, size_t size, size_t offset) override;

		OpenGL& push_debug_stage(const char* stage, const Color& color = {}) override;
		OpenGL& pop_debug_stage() override;

		void reset_samplers();

		~OpenGL();
	};
}// namespace Engine
