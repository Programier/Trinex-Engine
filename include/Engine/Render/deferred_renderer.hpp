#pragma once
#include <Core/etl/vector.hpp>
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
		FrameVector<PrimitiveComponent*> m_visible_primitives;
		FrameVector<LightComponent*> m_visible_lights;

	private:
		DeferredRenderer& geometry_pass();
		DeferredRenderer& deferred_lighting_pass();
		DeferredRenderer& copy_base_color_to_scene_color();
		DeferredRenderer& render_visible_primitives(RenderPass* pass);

	public:
		static Pipelines::DeferredLightPipeline* static_find_light_pipeline(LightComponent* component);

		DeferredRenderer(Scene* scene, const SceneView& view, ViewMode mode);
		trinex_non_copyable(DeferredRenderer);
		trinex_non_moveable(DeferredRenderer);

		DeferredRenderer& render() override;

		inline const FrameVector<PrimitiveComponent*>& visible_primitives() const { return m_visible_primitives; }
		inline const FrameVector<LightComponent*>& visible_lights() const { return m_visible_lights; }
	};
}// namespace Engine
