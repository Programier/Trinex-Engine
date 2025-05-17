#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	class Renderer;
	class Actor;

	namespace EditorRenderer
	{
		void render_grid(Renderer* renderer);
		void render_outlines(Renderer* renderer, Actor** actor, size_t count);
	}// namespace EditorRenderer
}// namespace Engine
