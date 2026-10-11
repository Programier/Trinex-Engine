#pragma once
#include <Core/etl/vector.hpp>

namespace Trinex
{
	class InputDevice;
}// namespace Trinex

namespace Trinex::Platform
{
	class ENGINE_EXPORT InputSystem
	{
	public:
		static InputSystem* instance();

		virtual ~InputSystem() = default;

		virtual bool devices(Vector<InputDevice>* out) const  = 0;
		virtual bool text_input_enabled() const               = 0;
		virtual InputSystem* text_input_enabled(bool enabled) = 0;
	};
}// namespace Trinex::Platform
