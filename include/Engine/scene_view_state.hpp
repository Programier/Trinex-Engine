#pragma once
#include <Core/math/vector.hpp>
#include <Engine/camera_view.hpp>

namespace Trinex
{
	class RHITexture;
	class RHIBuffer;
	class RHIContext;
	class SceneView;

	class ENGINE_EXPORT SceneViewState
	{
	private:
		CameraView m_view;
		Vector2u m_size           = {0u, 0u};
		RHITexture* m_scene_color = nullptr;

	private:
		SceneViewState& release();
		SceneViewState& allocate(RHIContext* ctx, Vector2u size);

	public:
		~SceneViewState();

		SceneViewState& resize(RHIContext* ctx, Vector2u size);
		SceneViewState& store(const SceneView& view);

		inline const CameraView& camera_view() const { return m_view; }
		inline RHITexture* scene_color() const { return m_scene_color; }
	};
}// namespace Trinex
