#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/structures.hpp>

namespace Engine
{
	struct WindowInterface;
	class Window;
	struct WindowConfig;
	class Event;

	class ENGINE_EXPORT WindowManager final : public Singletone<WindowManager, EmptyClass>
	{
	private:
		static WindowManager* m_instance;
		TreeMap<Identifier, Window*> m_windows;
		Window* m_main_window = nullptr;

		WindowManager();

	public:
		~WindowManager();
		Window* create_window(const WindowConfig& config, Window* parent = nullptr, Window* self = nullptr);
		WindowManager& destroy_window(Window* window);

		WindowManager& mouse_relative_mode(bool flag);
		WindowManager& pool_events(void (*callback)(const Event& event, void* userdata), void* userdata = nullptr);
		WindowManager& wait_for_events(void (*callback)(const Event& event, void* userdata), void* userdata = nullptr);
		Window* find(Identifier id) const;

		Window* main_window() const;
		const TreeMap<Identifier, Window*>& windows() const;
		friend class Window;
		friend class Singletone<WindowManager, EmptyClass>;
	};
}// namespace Engine
