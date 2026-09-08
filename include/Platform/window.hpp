#pragma once
#include <Core/enums.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>
#include <Core/math/vector.hpp>
#include <Platform/types.hpp>

namespace Trinex
{
	struct WindowDesc;
	class Window;
}// namespace Trinex

namespace Trinex::Platform
{
	class ENGINE_EXPORT WindowSystem
	{
	public:
		static WindowSystem* instance();

		virtual ~WindowSystem() = default;

		virtual Window* create_window(const Trinex::WindowDesc* desc) = 0;
		virtual void destroy_window(Window* window)                   = 0;
		virtual Window* main_window() const                           = 0;
		virtual Window* find_window(Identifier id) const              = 0;
		virtual bool mouse_relative_mode() const                      = 0;
		virtual WindowSystem* mouse_relative_mode(bool enabled)       = 0;
	};
}// namespace Trinex::Platform
