#include <Core/localization.hpp>
#include <UI/api.hpp>
#include <UI/menu_bar.hpp>

namespace Trinex::UI
{
	MenuBar::Menu& MenuBar::Menu::enabled(Function<bool()> callback)
	{
		is_enabled = std::move(callback);
		return *this;
	}

	MenuBar::Menu& MenuBar::Menu::enabled(bool value)
	{
		return enabled([value]() { return value; });
	}

	MenuBar::Menu& MenuBar::Menu::add_action(const Function<void()>& action)
	{
		actions.push(action);
		return *this;
	}

	MenuBar::Menu& MenuBar::Menu::add_action(Function<void()>&& action)
	{
		actions.push(std::move(action));
		return *this;
	}

	MenuBar::Menu& MenuBar::Menu::clear_actions()
	{
		actions.clear();
		return *this;
	}

	bool MenuBar::is_attached(const Menu* menu) const
	{
		return menu && (menu == m_first || menu == m_last || menu->m_prev || menu->m_next);
	}

	void MenuBar::append(Menu* menu)
	{
		if (m_last)
		{
			m_last->m_next = menu;
			menu->m_prev   = m_last;
			m_last         = menu;
		}
		else
		{
			m_first = menu;
			m_last  = menu;
		}
	}

	void MenuBar::insert_before(Menu* menu, Menu* before)
	{
		if (before == nullptr)
		{
			append(menu);
			return;
		}

		menu->m_prev = before->m_prev;
		menu->m_next = before;

		if (before->m_prev)
			before->m_prev->m_next = menu;
		else
			m_first = menu;

		before->m_prev = menu;
	}

	void MenuBar::remove(Menu* menu)
	{
		if (!is_attached(menu))
			return;

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


	MenuBar::Menu* MenuBar::create(StringView name, Menu* before)
	{
		if (auto it = m_menus.find(String(name)); it != m_menus.end())
			return it->second;

		Menu* menu = trx_new Menu(this, name);
		m_menus.emplace(menu->name, menu);

		if (before && before->bar == this)
		{
			place_before(menu, before);
		}
		else
		{
			append(menu);
		}

		return menu;
	}

	MenuBar::Menu& MenuBar::menu(StringView name, Menu* before)
	{
		return *create(name, before);
	}

	MenuBar::Menu& MenuBar::menu(StringView name, const Function<void()>& action, Menu* before)
	{
		return menu(name, before).add_action(action);
	}

	MenuBar::Menu& MenuBar::menu(StringView name, Function<void()>&& action, Menu* before)
	{
		return menu(name, before).add_action(std::move(action));
	}

	MenuBar::Menu* MenuBar::find(StringView name) const
	{
		if (auto it = m_menus.find(String(name)); it != m_menus.end())
			return it->second;
		return nullptr;
	}

	bool MenuBar::place_before(Menu* src, Menu* before)
	{
		if (!src || src == before)
			return false;

		if (src->bar != this || (before && before->bar != this))
			return false;

		if (before == nullptr)
		{
			if (src == m_last)
				return true;
		}
		else if (src->m_next == before)
		{
			return true;
		}

		remove(src);
		insert_before(src, before);

		return true;
	}

	bool MenuBar::place_after(Menu* src, Menu* after)
	{
		return place_before(src, after ? after->m_next : m_first);
	}

	MenuBar& MenuBar::destroy(Menu* menu)
	{
		if (menu && menu->bar == this)
		{
			remove(menu);
			m_menus.erase(menu->name);
			trx_delete_inline(menu);
		}
		return *this;
	}

	MenuBar& MenuBar::render()
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

				if (UI::begin_menu(localization->localize(menu->name).c_str(), enabled))
				{
					menu->actions();
					UI::end_menu();
				}
			}
		}
		return *this;
	}

	MenuBar& MenuBar::clear()
	{
		while (m_first)
		{
			Menu* next = m_first->m_next;
			trx_delete_inline(m_first);
			m_first = next;
		}

		m_menus.clear();
		m_last = nullptr;
		return *this;
	}

	MenuBar::~MenuBar()
	{
		clear();
	}
}// namespace Trinex::UI
