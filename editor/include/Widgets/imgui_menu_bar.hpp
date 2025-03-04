#pragma once
#include <Core/callback.hpp>
#include <Core/etl/string.hpp>

namespace Engine
{
	class ImGuiMenuBar final
	{
	public:
		struct Menu {
			ImGuiMenuBar* const bar;
			const String name;
			Function<bool()> is_enabled;
			CallBacks<void()> actions;

		private:
			Menu* m_next = nullptr;
			Menu* m_prev = nullptr;

			FORCE_INLINE Menu(ImGuiMenuBar* bar, StringView name) : bar(bar), name(name) {}
			FORCE_INLINE ~Menu() {}

		public:
			FORCE_INLINE bool place_before(Menu* before) { return bar->place_before(this, before); }
			FORCE_INLINE bool place_after(Menu* after) { return bar->place_after(this, after); }
			FORCE_INLINE Menu* next() const { return m_next; }
			FORCE_INLINE Menu* prev() const { return m_prev; }
			FORCE_INLINE void destroy() { bar->destroy(this); }

			friend class ImGuiMenuBar;
		};

	private:
		Menu* m_first = nullptr;
		Menu* m_last  = nullptr;

		void remove(Menu* menu);

	public:
		FORCE_INLINE ImGuiMenuBar() = default;
		delete_copy_constructors(ImGuiMenuBar);

		FORCE_INLINE bool is_empty() const { return m_first == nullptr; }
		FORCE_INLINE Menu* first() const { return m_first; }
		FORCE_INLINE Menu* last() const { return m_last; }

		Menu* create(StringView name, Menu* before = nullptr);
		Menu* find(StringView name) const;
		bool place_before(Menu* src, Menu* before);
		bool place_after(Menu* src, Menu* after);
		ImGuiMenuBar& destroy(Menu* menu);
		ImGuiMenuBar& render();
		ImGuiMenuBar& clear();
		~ImGuiMenuBar();
	};
}// namespace Engine
