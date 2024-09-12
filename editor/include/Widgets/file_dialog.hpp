#pragma once
#include <Graphics/imgui.hpp>

namespace Engine
{
	class ImGuiFileDialog : public ImGuiWidget
	{
		Path m_path;
		Vector<String> m_splited_path;

		ImGuiFileDialog& setup_dock_space();
		ImGuiFileDialog& render_header();
		ImGuiFileDialog& render_left_panel();
		ImGuiFileDialog& render_content();
		ImGuiFileDialog& render_footer();

		ImGuiFileDialog& change_path(const Path& new_path);

	public:
		String name;

		void init(RenderViewport* viewport) override;
		bool render(RenderViewport* viewport) override;
	};
}// namespace Engine
