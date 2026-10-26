#pragma once
#include <Core/etl/delegate.hpp>
#include <Core/ref_counted.hpp>
#include <Input/types.hpp>

namespace Trinex
{
	class InputDevice : public RefCounted
	{
	public:
		MulticastDelegate<void(const InputEvent&)> on_event;

	public:
		virtual const char* name() const                  = 0;
		virtual u32 types() const                         = 0;
		virtual InputDeviceType type(u32 index = 0) const = 0;
		virtual const InputEvent* state(u32& len) const   = 0;
	};
} // namespace Trinex
