#pragma once
#include <Core/enums.hpp>
#include <Core/ref_counted.hpp>

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

		virtual Ref<Window> create(const char* title, u32 width, u32 height, WindowAttribute flags) = 0;
		virtual Ref<Window> grabbed() const                                                         = 0;
		virtual Ref<Window> find(u32 id) const                                                      = 0;
	};
}// namespace Trinex::Platform
