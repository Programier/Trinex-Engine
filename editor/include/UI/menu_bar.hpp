#pragma once
#include <Core/callback.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/string.hpp>

namespace Trinex::UI
{
	class MenuBar final
	{
	public:
		struct Menu {
			MenuBar* const bar;
			const String name;
			Function<bool()> is_enabled;
			CallBacks<void()> actions;

		private:
			Menu* m_next = nullptr;
			Menu* m_prev = nullptr;

			FORCE_INLINE Menu(MenuBar* bar, StringView name) : bar(bar), name(name) {}
			FORCE_INLINE ~Menu() {}

		public:
			FORCE_INLINE bool place_before(Menu* before) { return bar->place_before(this, before); }
			FORCE_INLINE bool place_after(Menu* after) { return bar->place_after(this, after); }
			FORCE_INLINE Menu* next() const { return m_next; }
			FORCE_INLINE Menu* prev() const { return m_prev; }
			FORCE_INLINE bool is_root() const { return name.empty(); }
			FORCE_INLINE void destroy() { bar->destroy(this); }
			Menu& enabled(Function<bool()> callback);
			Menu& enabled(bool value);
			Menu& add_action(const Function<void()>& action);
			Menu& add_action(Function<void()>&& action);
			Menu& clear_actions();

			friend class MenuBar;
		};

	private:
		Menu* m_first = nullptr;
		Menu* m_last  = nullptr;
		Map<String, Menu*> m_menus;

		bool is_attached(const Menu* menu) const;
		void append(Menu* menu);
		void insert_before(Menu* menu, Menu* before);
		void remove(Menu* menu);

	public:
		FORCE_INLINE MenuBar() = default;
		delete_copy_constructors(MenuBar);

		FORCE_INLINE bool is_empty() const { return m_first == nullptr; }
		FORCE_INLINE Menu* first() const { return m_first; }
		FORCE_INLINE Menu* last() const { return m_last; }

		Menu* create(StringView name, Menu* before = nullptr);
		Menu& menu(StringView name, Menu* before = nullptr);
		Menu& menu(StringView name, const Function<void()>& action, Menu* before = nullptr);
		Menu& menu(StringView name, Function<void()>&& action, Menu* before = nullptr);
		Menu* find(StringView name) const;
		bool place_before(Menu* src, Menu* before);
		bool place_after(Menu* src, Menu* after);
		MenuBar& destroy(Menu* menu);
		MenuBar& render();
		MenuBar& clear();
		~MenuBar();
	};
}// namespace Trinex::UI
