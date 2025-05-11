#pragma once
#include <Engine/Render/renderer.hpp>

namespace Engine
{
	class ENGINE_EXPORT DeferredRenderer : public Renderer
	{
	private:
		void geometry_pass();
		void ambient_pass();
		void copy_base_color_to_scene_color();

	public:
		DeferredRenderer(Scene* scene, const SceneView& view, ViewMode mode);
		trinex_non_copyable(DeferredRenderer);
		trinex_non_moveable(DeferredRenderer);

		DeferredRenderer& render(RenderGraph::Graph& graph);
	};
}// namespace Engine
