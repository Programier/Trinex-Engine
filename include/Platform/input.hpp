#pragma once
#include <Core/etl/array.hpp>
#include <Core/ref_counted.hpp>

namespace Trinex
{
	class InputDevice;
}// namespace Trinex

namespace Trinex::Platform
{
	class trinex_api InputSystem
	{
	public:
		static InputSystem* instance();

		virtual ~InputSystem() = default;

		virtual usize devices() const                        = 0;
		virtual WeakRef<InputDevice> device(usize idx) const = 0;

		virtual bool text_input_enabled() const               = 0;
		virtual InputSystem& text_input_enabled(bool enabled) = 0;
	};
}// namespace Trinex::Platform
