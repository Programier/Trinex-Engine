#include <Core/localization.hpp>
#include <Widgets/imgui_menu_bar.hpp>
#include <imgui.h>

namespace Engine
{
	void ImGuiMenuBar::remove(Menu* menu)
	{
		if (menu == m_first)
			m_first = menu->m_next;

		if (menu == m_last)
			m_last = menu->m_prev;

		if (menu->m_prev)
			menu->m_prev->m_next = menu->m_next;

		if (menu->m_next)
			menu->m_next->m_prev = menu->m_prev;

		menu->m_prev = nullptr;
		menu->m_next = nullptr;
	}


	ImGuiMenuBar::Menu* ImGuiMenuBar::create(StringView name, Menu* before)
	{
		if (auto menu = find(name))
			return menu;

		Menu* menu = trx_new Menu(this, name);

		if (before && before->bar == this)
		{
			place_before(menu, before);
		}
		else
		{
			if (m_first == nullptr)
			{
				m_first = menu;
				m_last  = menu;
			}
			else
			{
				m_last->m_next = menu;
				menu->m_prev   = m_last;
				m_last         = menu;
			}
		}

		return menu;
	}

	ImGuiMenuBar::Menu* ImGuiMenuBar::find(StringView name) const
	{
		Menu* menu = m_first;

		while (menu && menu->name != name)
		{
			menu = menu->m_next;
		}

		return nullptr;
	}

	bool ImGuiMenuBar::place_before(Menu* src, Menu* before)
	{
		if (!src || src == before)
			return false;

		if (src->bar != this || (before && before->bar != this))
			return false;

		if (before == nullptr)
		{
			if (src == m_last)
				return true;

			remove(src);
			m_last->m_next = src;
			src->m_prev    = m_last;
			m_last         = src;
		}
		else
		{
			remove(src);

			src->m_prev = before->m_prev;
			src->m_next = before;

			if (before->m_prev)
				before->m_prev->m_next = src;

			before->m_prev = src;

			if (before == m_first)
				m_first = src;
		}

		return true;
	}

	bool ImGuiMenuBar::place_after(Menu* src, Menu* after)
	{
		return place_before(src, after ? after->m_next : m_first);
	}

	ImGuiMenuBar& ImGuiMenuBar::destroy(Menu* menu)
	{
		if (menu && menu->bar == this)
		{
			remove(menu);
			trx_delete_inline(menu);
		}
		return *this;
	}

	ImGuiMenuBar& ImGuiMenuBar::render()
	{
		Localization* localization = Localization::instance();

		for (Menu* menu = m_first; menu; menu = menu->m_next)
		{
			if (menu->name.empty())
			{
				menu->actions();
			}
			else
			{
				bool enabled = true;

				if (menu->is_enabled)
				{
					enabled = menu->is_enabled();
				}

				if (ImGui::BeginMenu(localization->localize(menu->name).c_str(), enabled))
				{
					menu->actions();
					ImGui::EndMenu();
				}
			}
		}
		return *this;
	}

	ImGuiMenuBar& ImGuiMenuBar::clear()
	{
		while (m_first)
		{
			Menu* next = m_first->m_next;
			trx_delete_inline(m_first);
			m_first = next;
		}

		m_last = nullptr;
		return *this;
	}

	ImGuiMenuBar::~ImGuiMenuBar()
	{
		clear();
	}
}// namespace Engine
