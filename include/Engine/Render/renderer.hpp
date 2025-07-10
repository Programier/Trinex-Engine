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
	struct RHI_Buffer;
	struct RHI_Texture;

	namespace RenderGraph
	{
		class Graph;
	}

	struct ENGINE_EXPORT RenderStatistics final {
		size_t visible_objects;

		FORCE_INLINE RenderStatistics& reset()
		{
			visible_objects = 0;
			return *this;
		}
	};

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
			SceneColorHDR, /**< Render target for scene hdr colors */
			SceneColorLDR, /**< Render target for scene ldr colors */
			SceneDepth,    /**< Render target for scene depths */
			BaseColor,     /**< Render target for base color */
			Normal,        /**< Render target for normal */
			Emissive,      /**< Render target for emissive */
			MSRA,          /**< Render target for MSRA */

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
		RHI_Buffer* m_globals = nullptr;
		SceneView m_view;
		ViewMode m_view_mode;
		RHI_Texture* m_surfaces[LastSurface] = {};

	public:
		static RHISurfaceFormat format_of(SurfaceType type);
		static Renderer* static_create_renderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit);

	public:
		BatchedLines lines;
		RenderStatistics statistics;

		Renderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit);
		trinex_non_copyable(Renderer);
		trinex_non_moveable(Renderer);

		Renderer& render_primitive(RenderPass* pass, PrimitiveComponent* component, const MaterialBindings* bindings = nullptr);
		Renderer& add_child_renderer(Renderer* renderer);
		RHI_Texture* surface(SurfaceType type);
		RHI_Texture* scene_color_target();
		RHI_Buffer* globals_uniform_buffer();

		virtual Renderer& render();

		inline const SceneView& scene_view() const { return m_view; }
		inline Scene* scene() const { return m_scene; }
		inline ViewMode view_mode() const { return m_view_mode; }
		inline RHI_Texture* scene_color_hdr_target() { return surface(SceneColorHDR); }
		inline RHI_Texture* scene_color_ldr_target() { return surface(SceneColorLDR); }
		inline RHI_Texture* scene_depth_target() { return surface(SceneDepth); }
		inline RHI_Texture* base_color_target() { return surface(BaseColor); }
		inline RHI_Texture* normal_target() { return surface(Normal); }
		inline RHI_Texture* emissive_target() { return surface(Emissive); }
		inline RHI_Texture* msra_target() { return surface(MSRA); }
		inline RenderGraph::Graph* render_graph() const { return m_graph; }

		virtual ~Renderer() {}
	};

}// namespace Engine
