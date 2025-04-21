#include <Core/editor_config.hpp>
#include <Core/editor_resources.hpp>
#include <Graphics/material.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	void render_editor_grid(const struct CameraView& view, RenderPass* pass)
	{
		if (Settings::Editor::show_grid == false)
			return;

		if (EditorResources::grid_material->apply(nullptr, pass))
			rhi->draw(6, 0);
	}
}// namespace Engine
