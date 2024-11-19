#pragma once
#include <Engine/Render/scene_renderer.hpp>

namespace Engine
{
	class EditorOverlayPass;

	class EditorSceneRenderer : public ColorSceneRenderer
	{
	private:
		EditorOverlayPass* m_overlay_pass;

	public:
		EditorSceneRenderer();

		// Components rendering
		using ColorSceneRenderer::render_component;
		EditorSceneRenderer& render_component(LightComponent* component) override;
		EditorSceneRenderer& render_component(PrimitiveComponent* component) override;
	};
}// namespace Engine
