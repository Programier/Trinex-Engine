#pragma once
#include <Core/callback.hpp>
#include <UI/types.hpp>

namespace Trinex
{
	class Package;

	class ContentBrowserWidget : public UI::Widget
	{
	private:
		void selecte_new_object(Object* object);
		void begin_renaming(Object* object = nullptr);

		bool render_package_popup(void* data);
		void render_package_tree(Package* node);
		void render_package_popup();
		void render_packages();

		bool render_content_item(Object* object, const UI::Vec2& size);
		void render_content_window();

		String m_new_object_name;
		Package* m_show_popup_for   = nullptr;
		Package* m_selected_package = nullptr;
		UI::ID m_dock_window_id;
		bool m_is_renaming : 1 = false;


	public:
		CallBacks<void(Object*)> on_object_select;
		CallBacks<void(Object*)> on_object_double_click;
		CallBacks<bool(Refl::Class*)> filters;

		class Object* selected_object = nullptr;

		ContentBrowserWidget();
		void on_render() override;
		Package* selected_package() const;
		static const char* static_name();
		~ContentBrowserWidget();
	};
}// namespace Trinex
