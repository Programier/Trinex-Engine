#pragma once
#include <Core/callback.hpp>
#include <Core/etl/string.hpp>

namespace Trinex
{
	class ImGuiMenuBar final
	{
	public:
		struct Menu {
			ImGuiMenuBar* const bar;
			const String name;
			Delegate<bool()> is_enabled;
			CallBacks<void()> actions;

		private:
			Menu* m_next = nullptr;
			Menu* m_prev = nullptr;

			inline Menu(ImGuiMenuBar* bar, StringView name) : bar(bar), name(name) {}
			inline ~Menu() {}

		public:
			inline bool place_before(Menu* before) { return bar->place_before(this, before); }
			inline bool place_after(Menu* after) { return bar->place_after(this, after); }
			inline Menu* next() const { return m_next; }
			inline Menu* prev() const { return m_prev; }
			inline void destroy() { bar->destroy(this); }

			friend class ImGuiMenuBar;
		};

	private:
		Menu* m_first = nullptr;
		Menu* m_last  = nullptr;

		void remove(Menu* menu);

	public:
		inline ImGuiMenuBar() = default;

		trinex_non_copyable(ImGuiMenuBar);
		trinex_non_moveable(ImGuiMenuBar);

		inline bool is_empty() const { return m_first == nullptr; }
		inline Menu* first() const { return m_first; }
		inline Menu* last() const { return m_last; }

		Menu* create(StringView name, Menu* before = nullptr);
		Menu* find(StringView name) const;
		bool place_before(Menu* src, Menu* before);
		bool place_after(Menu* src, Menu* after);
		ImGuiMenuBar& destroy(Menu* menu);
		ImGuiMenuBar& render();
		ImGuiMenuBar& clear();
		~ImGuiMenuBar();
	};
}// namespace Trinex
