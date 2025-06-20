#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/singletone.hpp>

namespace Engine
{
	struct WindowInterface;
	class Window;
	struct WindowConfig;
	struct Event;

	class ENGINE_EXPORT WindowManager final : public Singletone<WindowManager, EmptySingletoneParent>
	{
	private:
		static WindowManager* s_instance;
		TreeMap<Identifier, Window*> m_windows;
		Window* m_main_window = nullptr;

		WindowManager();

	public:
		~WindowManager();
		Window* create_window(const WindowConfig& config, Window* parent = nullptr, Window* self = nullptr);
		WindowManager& destroy_window(Window* window);
		WindowManager& mouse_relative_mode(bool flag);
		Window* find(Identifier id) const;
		Window* main_window() const;
		const TreeMap<Identifier, Window*>& windows() const;
		friend class Window;
		friend class Singletone<WindowManager, EmptySingletoneParent>;
	};
}// namespace Engine
