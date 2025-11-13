#pragma once
#include <Core/etl/vector.hpp>
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
	class RHIContext;

	namespace RenderGraph
	{
		class Graph;
		class Pass;
	}// namespace RenderGraph

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
		RenderGraph::Pass* m_surface_clears[LastSurface] = {};

	public:
		static RHISurfaceFormat static_surface_format_of(SurfaceType type);
		static const char* static_surface_name_of(SurfaceType type);
		static void static_sort_lights(FrameVector<LightComponent*>& visible_lights);

	public:
		BatchedLines lines;

		Renderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit);
		trinex_non_copyable(Renderer);
		trinex_non_moveable(Renderer);

		Renderer& add_child_renderer(Renderer* renderer);
		RHITexture* surface(SurfaceType type);
		RenderGraph::Pass* surface_clear_pass(SurfaceType type);
		RHIBuffer* globals_uniform_buffer();

		virtual Renderer& render(RHIContext* ctx);
		virtual Renderer& reset(const SceneView& view);

		inline const SceneView& scene_view() const { return m_view; }
		inline Scene* scene() const { return m_scene; }
		inline ViewMode view_mode() const { return m_view_mode; }

		inline RHITexture* scene_color_hdr_target() { return surface(SceneColorHDR); }
		inline RHITexture* scene_color_ldr_target() { return surface(SceneColorLDR); }
		inline RHITexture* scene_depth_target() { return surface(SceneDepth); }
		inline RHITexture* base_color_target() { return surface(BaseColor); }
		inline RHITexture* normal_target() { return surface(Normal); }
		inline RHITexture* msra_target() { return surface(MSRA); }
		inline RHITexture* velocity_target() { return surface(Velocity); }

		inline RenderGraph::Pass* scene_color_hdr_clear_pass() { return surface_clear_pass(SceneColorHDR); }
		inline RenderGraph::Pass* scene_color_ldr_clear_pass() { return surface_clear_pass(SceneColorLDR); }
		inline RenderGraph::Pass* scene_depth_clear_pass() { return surface_clear_pass(SceneDepth); }
		inline RenderGraph::Pass* base_color_clear_pass() { return surface_clear_pass(BaseColor); }
		inline RenderGraph::Pass* normal_clear_pass() { return surface_clear_pass(Normal); }
		inline RenderGraph::Pass* msra_clear_pass() { return surface_clear_pass(MSRA); }
		inline RenderGraph::Pass* velocity_clear_pass() { return surface_clear_pass(Velocity); }

		inline RenderGraph::Graph* render_graph() const { return m_graph; }

		virtual ~Renderer() {}
	};

}// namespace Engine
