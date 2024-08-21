#pragma once
#include <Core/callback.hpp>
#include <Graphics/imgui.hpp>

namespace Engine
{
	class ContentBrowser : public ImGuiRenderer::ImGuiAdditionalWindow
	{
	private:
		bool render_package_popup(void* data);
		void render_package_tree(Package* node);
		void render_package_popup();
		void render_packages();
		bool show_context_menu(void* userdata);

		bool render_content_item(Object* object, const StringView& name, const ImVec2& item_size, const ImVec2& content_size,
		                         bool& not_first_item);
		void render_content_window();

		void create_dock_space();

		Package* m_show_popup_for   = nullptr;
		Package* m_selected_package = nullptr;

		bool m_show_context_menu = false;

		ImGuiID m_dock_window_id;


	public:
		CallBacks<void(Object*)> on_object_select;
		CallBacks<void(Object*)> on_object_double_click;
		CallBacks<bool(class Class*)> filters;

		class Object* selected_object = nullptr;

		void init(RenderViewport* viewport) override;
		bool render(RenderViewport* viewport) override;
		Package* selected_package() const;

		static const char* name();
		~ContentBrowser();
	};
}// namespace Engine
