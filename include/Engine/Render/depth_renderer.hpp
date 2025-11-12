#pragma once
#include <Engine/Render/renderer.hpp>

namespace Engine
{
	struct CameraView;

	class ENGINE_EXPORT DepthRenderer : public Renderer
	{
	private:
		DepthRenderer& render_depth(RHIContext* ctx);

	public:
		DepthRenderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit);
	};

	class ENGINE_EXPORT DepthCubeRenderer : public Renderer
	{
	private:
		RHITexture* m_cubemap = nullptr;

	private:
		DepthCubeRenderer& clear_depth(RHIContext* ctx);
		DepthCubeRenderer& render_depth(RHIContext* ctx, const Vector3f& forward, const Vector3f& up, uint_t face);
		DepthCubeRenderer& render_depth(RHIContext* ctx);

	public:
		DepthCubeRenderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit);
		DepthCubeRenderer& render(RHIContext* cxt) override;

		inline RHITexture* cubemap() const { return m_cubemap; }
	};
}// namespace Engine
