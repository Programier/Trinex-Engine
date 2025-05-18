#pragma once
#include <Engine/Render/renderer.hpp>


namespace Engine
{
	namespace Pipelines
	{
		class DeferredLightPipeline;
	}

	class ENGINE_EXPORT DeferredRenderer : public Renderer
	{
	private:
		void geometry_pass();
		void deferred_lighting_pass();
		void copy_base_color_to_scene_color();

	public:
		static Pipelines::DeferredLightPipeline* static_find_light_pipeline(LightComponent* component);

		DeferredRenderer(Scene* scene, const SceneView& view, ViewMode mode);
		trinex_non_copyable(DeferredRenderer);
		trinex_non_moveable(DeferredRenderer);

		DeferredRenderer& render(RenderGraph::Graph& graph);
	};
}// namespace Engine
