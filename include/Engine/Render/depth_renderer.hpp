#pragma once
#include <Engine/Render/renderer.hpp>

namespace Engine
{
	struct CameraView;

	class ENGINE_EXPORT DepthRenderer : public Renderer
	{
	private:
		DepthRenderer& render_depth();

	public:
		DepthRenderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit);
	};

	class ENGINE_EXPORT DepthCubeRenderer : public Renderer
	{
	private:
		RHITexture* m_cubemap = nullptr;

	private:
		DepthCubeRenderer& render_depth(CameraView& camera, uint_t face);
		DepthCubeRenderer& render_depth();

	public:
		DepthCubeRenderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit);

		inline RHITexture* cubemap() const { return m_cubemap; }
	};
}// namespace Engine
