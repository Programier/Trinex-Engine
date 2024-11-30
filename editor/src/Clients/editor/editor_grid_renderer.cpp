#include <Core/editor_config.hpp>
#include <Core/editor_resources.hpp>
#include <Graphics/material.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	void render_editor_grid(const struct CameraView& view)
	{
		if (Settings::Editor::show_grid == false)
			return;

		EditorResources::grid_material->apply();
		rhi->draw(6, 0);
	}
}// namespace Engine
