#pragma once

#include <d3d11.h>
#include <d3d11_definitions.hpp>
#include <d3d11_uniform_buffer.hpp>
#include <none_api.hpp>

namespace Engine
{
#define DXAPI Engine::D3D11::m_instance
	template<typename T>
	FORCE_INLINE void d3d11_release(T*& address)
	{
		if (address)
		{
			address->Release();
			address = nullptr;
		}
	}

	enum class D3D11_ViewportMode
	{
		Undefined = 0,
		Normal    = 1,
	};

	struct D3D11_State {
		ViewPort viewport;
		Scissor scissor;
		Vector2D render_target_size      = {-1, -1};
		class D3D11_Pipeline* pipeline   = nullptr;
		D3D11_ViewportMode viewport_mode = D3D11_ViewportMode::Undefined;

		D3D11_State();
		void reset();
	};

	class D3D11 : public NoneApi
	{
	public:
		declare_struct(D3D11, void);

		static D3D11* static_constructor();
		static void static_destructor(D3D11* d3d11);

		static D3D11* m_instance;
		D3D11_State m_state;

		Window* m_main_window = nullptr;

		IDXGIFactory* m_dxgi_factory   = nullptr;
		IDXGIAdapter* m_dxgi_adapter   = nullptr;
		ID3D11Device* m_device         = nullptr;
		ID3D11DeviceContext* m_context = nullptr;
#if D3D11_WITH_DEBUG
		ID3D11Debug* m_debug = nullptr;
#endif

		D3D_FEATURE_LEVEL m_feature_level = D3D_FEATURE_LEVEL_11_0;

		D3D11_GlobalUniforms m_global_uniform_buffer;
		D3D11_LocalUniforms m_local_unifor_buffer;

		D3D11();

		ID3D11RenderTargetView* create_render_target_view(ID3D11Texture2D* buffer, DXGI_FORMAT format);
		ID3D11DepthStencilView* create_depth_stencil_view(ID3D11Texture2D* buffer, DXGI_FORMAT format);

		D3D11& push_global_params(const GlobalShaderParameters& params) override;
		D3D11& pop_global_params() override;
		D3D11& update_local_parameter(const void* data, size_t size, size_t offset) override;

		D3D11& initialize(Window* window) override;
		void* context() override;
		D3D11& submit() override;

		RHI_Viewport* create_viewport(WindowRenderViewport* viewport, bool vsync) override;
		D3D11& viewport(const ViewPort& viewport) override;
		ViewPort viewport() override;
		D3D11& scissor(const Scissor& scissor) override;
		Scissor scissor() override;

		D3D11& bind_render_target(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil) override;

		D3D11& prepare_draw();
		D3D11_ViewportMode current_viewport_mode();
		D3D11& draw(size_t vertex_count, size_t vertices_offset) override;
		D3D11& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) override;
		D3D11& draw_instanced(size_t vertex_count, size_t vertex_offset, size_t instances) override;
		D3D11& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset, size_t instances) override;

		RHI_Sampler* create_sampler(const Sampler*) override;
		RHI_Texture2D* create_texture_2d(const Texture2D*) override;
		RHI_Texture2D* create_render_surface(const RenderSurface* surface) override;

		RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data, RHIBufferType type) override;
		RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, IndexBufferFormat format, RHIBufferType type) override;
		RHI_SSBO* create_ssbo(size_t size, const byte* data, RHIBufferType type) override;
		RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;
		RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
		RHI_Shader* create_tesselation_control_shader(const TessellationControlShader* shader) override;
		RHI_Shader* create_tesselation_shader(const TessellationShader* shader) override;
		RHI_Shader* create_geometry_shader(const GeometryShader* shader) override;
		RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;

		D3D11& push_debug_stage(const char* stage, const Color& color = {}) override;
		D3D11& pop_debug_stage() override;
		~D3D11();
	};
}// namespace Engine
