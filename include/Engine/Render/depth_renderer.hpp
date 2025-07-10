#pragma once
#include <Engine/Render/renderer.hpp>

namespace Engine
{
	class ENGINE_EXPORT DepthRenderer : public Renderer
	{
	private:
		DepthRenderer& render_depth();

	public:
		DepthRenderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit);
	};
}// namespace Engine
