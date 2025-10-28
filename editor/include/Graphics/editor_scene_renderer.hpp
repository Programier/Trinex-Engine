#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/set.hpp>
#include <Core/math/vector.hpp>
#include <Engine/Render/deferred_renderer.hpp>

namespace Engine
{
	class Actor;

	class EditorRenderer : public DeferredRenderer
	{
	public:
		static Actor* static_raycast(const SceneView& view, Vector2f uv, Scene* scene);

	public:
		EditorRenderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit);

		EditorRenderer& render_grid();
		EditorRenderer& render_outlines(const Set<Actor*>& actors);
		EditorRenderer& render_primitives(const Set<Actor*>& actors);
	};
}// namespace Engine
