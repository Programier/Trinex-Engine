#pragma once
#include <Engine/Render/scene_renderer.hpp>

namespace Engine
{
	class OverlaySceneLayer;
	class EditorSceneRenderer : public ColorSceneRenderer
	{
	private:
		OverlaySceneLayer* m_overlay_layer;

	public:
		EditorSceneRenderer();

		// Components rendering
		EditorSceneRenderer& render_component(LightComponent* component) override;
		EditorSceneRenderer& render_component(PrimitiveComponent* component) override;
	};
}// namespace Engine
