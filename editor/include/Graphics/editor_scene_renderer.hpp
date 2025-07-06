#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	class Renderer;
	class Actor;
	class SceneView;
	class Scene;

	namespace EditorRenderer
	{
		void render_grid(Renderer* renderer);
		void render_outlines(Renderer* renderer, Actor** actors, size_t count);
		void render_primitives(Renderer* renderer, Actor** actors = nullptr, size_t count = 0);
		Actor* raycast(const SceneView& view, Vector2f uv, Scene* scene);
	}// namespace EditorRenderer
}// namespace Engine
