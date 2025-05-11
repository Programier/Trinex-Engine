#pragma once
#include <Core/etl/vector.hpp>
#include <Engine/scene_view.hpp>
#include <Graphics/types/color_format.hpp>

namespace Engine
{
	class Scene;
	class RenderSurface;
	class RenderPass;
	class PrimitiveComponent;
	struct RHI_Buffer;
	struct RHI_Texture2D;

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
		using CustomPass = void (*)(Renderer* renderer, RenderGraph::Graph&);

		enum Surface
		{
			SceneColor, /**< Render target for scene colors */
			SceneDepth, /**< Render target for scene depths */
			BaseColor,  /**< Render target for base color */
			Normal,     /**< Render target for normal */
			Emissive,   /**< Render target for emissive */
			MSRA,       /**< Render target for MSRA */

			LastSurface,
		};

	private:
		RHI_Buffer* m_globals;
		SceneView m_view;
		Scene* m_scene;
		ViewMode m_view_mode;

		RHI_Texture2D* m_surfaces[LastSurface];
		FrameVector<CustomPass> m_custom_passes;
		FrameVector<PrimitiveComponent*> m_visible_primitives;

		Renderer& register_clear_passes(RenderGraph::Graph& graph);

	public:
		static SurfaceFormat format_of(Surface type);
		static Renderer* static_create_renderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit);

	public:
		RenderStatistics statistics;

		Renderer(Scene* scene, const SceneView& view, ViewMode mode);
		trinex_non_copyable(Renderer);
		trinex_non_moveable(Renderer);

		Renderer& render();
		Renderer& render_visible_primitives(RenderPass* pass);
		virtual Renderer& render(RenderGraph::Graph& graph) = 0;

		inline RHI_Buffer* globals_uniform_buffer() { return m_globals; }
		inline const SceneView& scene_view() const { return m_view; }
		inline Scene* scene() const { return m_scene; }
		inline ViewMode view_mode() const { return m_view_mode; }
		inline const FrameVector<PrimitiveComponent*>& visible_primitives() const { return m_visible_primitives; }

		inline RHI_Texture2D* scene_color_target() { return m_surfaces[SceneColor]; }
		inline RHI_Texture2D* scene_depth_target() { return m_surfaces[SceneDepth]; }
		inline RHI_Texture2D* base_color_target() { return m_surfaces[BaseColor]; }
		inline RHI_Texture2D* normal_target() { return m_surfaces[Normal]; }
		inline RHI_Texture2D* emissive_target() { return m_surfaces[Emissive]; }
		inline RHI_Texture2D* msra_target() { return m_surfaces[MSRA]; }

		Renderer& add_custom_pass(CustomPass pass)
		{
			m_custom_passes.emplace_back(pass);
			return *this;
		}

		virtual ~Renderer() {}
	};

}// namespace Engine
