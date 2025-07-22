#pragma once
#include <Engine/Render/batched_primitives.hpp>
#include <Engine/scene_view.hpp>

namespace Engine
{
	class Scene;
	class RenderSurface;
	class RenderPass;
	class PrimitiveComponent;
	class LightComponent;
	class MaterialBindings;
	class RHIBuffer;
	class RHITexture;

	namespace RenderGraph
	{
		class Graph;
	}

	struct RendererContext {
		class Renderer* renderer;
		RenderPass* render_pass;
		Matrix4f local_to_world;

		inline RendererContext(Renderer* renderer = nullptr, RenderPass* render_pass = nullptr,
		                       const Matrix4f& local_to_world = Matrix4f(1.f))
		    : renderer(renderer), render_pass(render_pass), local_to_world(local_to_world)
		{}
	};

	class ENGINE_EXPORT Renderer
	{
	public:
		enum SurfaceType
		{
			SceneColorHDR,// Render target for scene hdr colors
			SceneColorLDR,// Render target for scene ldr colors
			SceneDepth,   // Render target for scene depths
			BaseColor,    // Render target for base color
			Normal,       // Render target for normal
			Emissive,     // Render target for emissive
			MSRA,         // Render target for MSRA (R: Metalic, G: Roughness, B: Specular, A: AO)
			Velocity,     // Render target for motion vectors

			LastSurface,
		};

	private:
		struct ChildRenderer {
			Renderer* renderer;
			ChildRenderer* next;
		};

		ChildRenderer* m_child_renderer = nullptr;
		RenderGraph::Graph* m_graph;
		Scene* m_scene;
		RHIBuffer* m_globals = nullptr;
		SceneView m_view;
		ViewMode m_view_mode;
		RHITexture* m_surfaces[LastSurface] = {};

	public:
		static RHISurfaceFormat static_surface_format_of(SurfaceType type);
		static const char* static_surface_name_of(SurfaceType type);
		static Renderer* static_create_renderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit);

	public:
		BatchedLines lines;

		Renderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit);
		trinex_non_copyable(Renderer);
		trinex_non_moveable(Renderer);

		Renderer& render_primitive(RenderPass* pass, PrimitiveComponent* component, const MaterialBindings* bindings = nullptr);
		Renderer& add_child_renderer(Renderer* renderer);
		RHITexture* surface(SurfaceType type);
		RHITexture* scene_color_target();
		RHIBuffer* globals_uniform_buffer();

		virtual Renderer& render();

		inline const SceneView& scene_view() const { return m_view; }
		inline Scene* scene() const { return m_scene; }
		inline ViewMode view_mode() const { return m_view_mode; }
		inline RHITexture* scene_color_hdr_target() { return surface(SceneColorHDR); }
		inline RHITexture* scene_color_ldr_target() { return surface(SceneColorLDR); }
		inline RHITexture* scene_depth_target() { return surface(SceneDepth); }
		inline RHITexture* base_color_target() { return surface(BaseColor); }
		inline RHITexture* normal_target() { return surface(Normal); }
		inline RHITexture* emissive_target() { return surface(Emissive); }
		inline RHITexture* msra_target() { return surface(MSRA); }
		inline RenderGraph::Graph* render_graph() const { return m_graph; }

		virtual ~Renderer() {}
	};

}// namespace Engine
