#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/vector.hpp>
#include <Engine/Render/deferred_renderer.hpp>

namespace Engine
{
	class Actor;

	class EditorRenderer : public DeferredRenderer
	{
	private:
		void render_outlines_pass(Actor** actors, size_t count);

	public:
		static Actor* static_raycast(const SceneView& view, Vector2f uv, Scene* scene);

	public:
		EditorRenderer(Scene* scene, const SceneView& view, ViewMode mode = ViewMode::Lit);

		EditorRenderer& render_grid();
		EditorRenderer& render_outlines(Actor** actors, size_t count);
		EditorRenderer& render_primitives(Actor** actors = nullptr, size_t count = 0);
	};
}// namespace Engine
