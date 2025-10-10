#pragma once
#include <Core/enums.hpp>
#include <Core/etl/pair.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>
#include <Core/math/vector.hpp>

namespace Engine
{
	class Window;
	struct WindowConfig;
	struct MonitorInfo;
	struct Rect2D;
	struct Event;
	class Path;

	namespace VFS
	{
		class FileSystem;
	}

	namespace Platform
	{
		struct ENGINE_EXPORT MonitorInfo {
			Vector2u pos;
			Vector2u size;
			float dpi;
		};

		ENGINE_EXPORT OperationSystemType system_type();
		ENGINE_EXPORT const char* system_name();
		ENGINE_EXPORT Path find_exec_directory();
		ENGINE_EXPORT size_t monitors_count();
		ENGINE_EXPORT MonitorInfo monitor_info(Index monitor_index = 0);

		namespace EventSystem
		{
			ENGINE_EXPORT void pool_events(void (*callback)(const Event& event, void* userdata), void* userdata = nullptr);
			ENGINE_EXPORT void wait_for_events(void (*callback)(const Event& event, void* userdata), void* userdata = nullptr);
		}// namespace EventSystem

		ENGINE_EXPORT VFS::FileSystem* create_filesystem(const Path& mount, const Path& path);

		namespace WindowManager
		{
			ENGINE_EXPORT void initialize();
			ENGINE_EXPORT void terminate();

			ENGINE_EXPORT Window* create_window(const WindowConfig* config);
			ENGINE_EXPORT void destroy_window(Window* interface);
			ENGINE_EXPORT bool mouse_relative_mode();
			ENGINE_EXPORT void mouse_relative_mode(bool flag);
		}// namespace WindowManager

		namespace LibraryLoader
		{
			ENGINE_EXPORT void* load_library(const String& name);
			ENGINE_EXPORT void close_library(void* handle);
			ENGINE_EXPORT void* find_function(void* handle, const String& name);
		}// namespace LibraryLoader
	}// namespace Platform
}// namespace Engine
