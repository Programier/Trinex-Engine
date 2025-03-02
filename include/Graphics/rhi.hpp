#pragma once
#include <Core/structures.hpp>

struct ImGuiContext;
struct ImDrawData;

namespace Engine
{
	class WindowRenderViewport;
	class SurfaceRenderViewport;
	struct WindowConfig;
	struct SamplerCreateInfo;

	class VertexShader;
	class FragmentShader;
	class TessellationControlShader;
	class TessellationShader;
	class GeometryShader;
	class Shader;
	class GraphicsPipeline;
	class Sampler;
	class RenderSurface;
	class Texture2D;
	struct GlobalShaderParameters;

	namespace Refl
	{
		class Struct;
	}

	struct ENGINE_EXPORT RHI_Object {
	protected:
		mutable size_t m_references;
		virtual void destroy() const = 0;

	public:
		RHI_Object(size_t init_ref_count = 1);
		virtual void add_reference() const;
		virtual void release() const;
		size_t references() const;
		virtual ~RHI_Object();
	};

	template<typename Base>
	struct RHI_DefaultDestroyable : public Base {
	protected:
		void destroy() const override
		{
			delete this;
		}
	};

	struct ENGINE_EXPORT RHI_BindingObject : RHI_Object {
		virtual void bind(BindLocation location) = 0;
	};

	struct ENGINE_EXPORT RHI_Sampler : RHI_BindingObject {
	};

	struct ENGINE_EXPORT RHI_Texture2D : RHI_BindingObject {
		virtual void bind_combined(RHI_Sampler* sampler, BindLocation location) = 0;

		// Surface only methods, calling these methods on non-surface objects will result in exceptions!
		virtual void clear_color(const Color& color);
		virtual void clear_depth_stencil(float depth, byte stencil);
		virtual void blit(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter);
	};

	struct ENGINE_EXPORT RHI_Shader : RHI_Object {
	};

	struct ENGINE_EXPORT RHI_Pipeline : RHI_Object {
		virtual void bind() = 0;
	};

	struct ENGINE_EXPORT RHI_Buffer : RHI_Object {
		virtual void update(size_t offset, size_t size, const byte* data) = 0;
	};

	struct ENGINE_EXPORT RHI_VertexBuffer : RHI_Buffer {
		virtual void bind(byte stream_index, size_t stride, size_t offset) = 0;
	};

	struct ENGINE_EXPORT RHI_IndexBuffer : RHI_Buffer {
		virtual void bind(size_t offset) = 0;
	};

	struct ENGINE_EXPORT RHI_SSBO : RHI_Buffer {
		virtual void bind(BindLocation location) = 0;
	};

	struct ENGINE_EXPORT RHI_UniformBuffer : RHI_Buffer {
		virtual void bind(BindingIndex location) = 0;
	};

	struct ENGINE_EXPORT RHI_Viewport : RHI_Object {
		virtual void present() = 0;

		virtual void vsync(bool flag)                                = 0;
		virtual void on_resize(const Size2D& new_size)               = 0;
		virtual void on_orientation_changed(Orientation orientation) = 0;
		virtual void bind()                                          = 0;
		virtual void blit_target(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
		                         SamplerFilter filter)               = 0;
		virtual void clear_color(const Color& color)                 = 0;
	};

	struct ENGINE_EXPORT RHI {
		struct Info {
			String name;
			String renderer;
			Refl::Struct* struct_instance = nullptr;
		} info;

		virtual RHI& initialize(class Window* window) = 0;
		virtual void* context()                       = 0;

		virtual RHI& draw(size_t vertex_count, size_t vertices_offset)                                 = 0;
		virtual RHI& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) = 0;
		virtual RHI& draw_instanced(size_t vertex_count, size_t vertex_offset, size_t instances)       = 0;
		virtual RHI& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
		                                    size_t instances)                                          = 0;

		virtual RHI& submit() = 0;

		virtual RHI& bind_render_target(const RenderSurface* rt1, const RenderSurface* rt2, const RenderSurface* rt3,
										const RenderSurface* rt4, RenderSurface* depth_stencil) = 0;

		inline RHI& bind_depth_stencil_target(RenderSurface* depth_stencil)
		{
			return bind_render_target(nullptr, nullptr, nullptr, nullptr, depth_stencil);
		}

		inline RHI& bind_render_target1(const RenderSurface* rt1, RenderSurface* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, nullptr, nullptr, nullptr, depth_stencil);
		}

		inline RHI& bind_render_target2(const RenderSurface* rt1, const RenderSurface* rt2,
										RenderSurface* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, rt2, nullptr, nullptr, depth_stencil);
		}

		inline RHI& bind_render_target3(const RenderSurface* rt1, const RenderSurface* rt2, const RenderSurface* rt3,
										RenderSurface* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, rt2, rt3, nullptr, depth_stencil);
		}

		inline RHI& bind_render_target4(const RenderSurface* rt1, const RenderSurface* rt2, const RenderSurface* rt3,
										const RenderSurface* rt4, RenderSurface* depth_stencil = nullptr)
		{
			return bind_render_target(rt1, rt2, rt3, rt4, depth_stencil);
		}

		virtual RHI& viewport(const ViewPort& viewport) = 0;
		virtual ViewPort viewport()                     = 0;
		virtual RHI& scissor(const Scissor& scissor)    = 0;
		virtual Scissor scissor()                       = 0;

		virtual RHI_Sampler* create_sampler(const Sampler*)                                                                  = 0;
		virtual RHI_Texture2D* create_texture_2d(const Texture2D*)                                                           = 0;
		virtual RHI_Texture2D* create_render_surface(const RenderSurface*)                                                   = 0;
		virtual RHI_Shader* create_vertex_shader(const VertexShader* shader)                                                 = 0;
		virtual RHI_Shader* create_tesselation_control_shader(const TessellationControlShader* shader)                       = 0;
		virtual RHI_Shader* create_tesselation_shader(const TessellationShader* shader)                                      = 0;
		virtual RHI_Shader* create_geometry_shader(const GeometryShader* shader)                                             = 0;
		virtual RHI_Shader* create_fragment_shader(const FragmentShader* shader)                                             = 0;
		virtual RHI_Pipeline* create_graphics_pipeline(const GraphicsPipeline* pipeline)                                     = 0;
		virtual RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data, RHIBufferType type)                    = 0;
		virtual RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, IndexBufferFormat format, RHIBufferType type) = 0;
		virtual RHI_SSBO* create_ssbo(size_t size, const byte* data, RHIBufferType type)                                     = 0;
		virtual RHI_UniformBuffer* create_uniform_buffer(size_t size, const byte* data, RHIBufferType type)                  = 0;
		virtual RHI_Viewport* create_viewport(WindowRenderViewport* viewport, bool vsync)                                    = 0;
		virtual RHI& update_scalar_parameter(const void* data, size_t size, size_t offset, BindingIndex buffer_index)        = 0;

		virtual RHI& push_debug_stage(const char* stage, const Color& color = {}) = 0;
		virtual RHI& pop_debug_stage()                                            = 0;

		virtual ~RHI() {};
	};

	ENGINE_EXPORT extern RHI* rhi;
}// namespace Engine
