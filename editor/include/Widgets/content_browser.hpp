#pragma once
#include <Core/callback.hpp>
#include <Graphics/imgui.hpp>

namespace Engine
{
	class ContentBrowser : public ImGuiWidget
	{
	private:
		void selecte_new_object(Object* object);
		void begin_renaming(Object* object = nullptr);

		bool render_package_popup(void* data);
		void render_package_tree(Package* node);
		void render_package_popup();
		void render_packages();

		bool render_content_item(Object* object, const ImVec2& item_size);
		void render_content_window();

		void create_dock_space();

		String m_new_object_name;
		Package* m_show_popup_for   = nullptr;
		Package* m_selected_package = nullptr;
		ImGuiID m_dock_window_id;
		bool m_is_renaming : 1 = false;


	public:
		CallBacks<void(Object*)> on_object_select;
		CallBacks<void(Object*)> on_object_double_click;
		CallBacks<bool(Refl::Class*)> filters;

		class Object* selected_object = nullptr;

		void init(RenderViewport* viewport) override;
		bool render(RenderViewport* viewport) override;
		Package* selected_package() const;

		virtual const char* name() const;
		static const char* static_name();
		~ContentBrowser();
	};
}// namespace Engine
