#include <Core/window.hpp>
#include <Platform/window.hpp>

namespace Trinex
{
	Ref<Window> Window::create(const char* title, Vector2u size, WindowAttribute attributes)
	{
		return Platform::WindowSystem::instance()->create(title, size.x, size.y, attributes);
	}

	Ref<Window> Window::find(u32 id)
	{
		return Platform::WindowSystem::instance()->find(id);
	}
} // namespace Trinex
