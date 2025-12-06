#pragma once
#include <Core/engine_types.hpp>

namespace Engine::Mouse
{
	struct Status {
		enum Enum : EnumerateType
		{
			Released = 0,
			JustReleased,
			JustPressed,
			Pressed,
		};

		trinex_enum_struct(Status);
		trinex_enum(Status);
	};

	struct Button {
		enum Enum : EnumerateType
		{
			Unknown,
			Left,
			Middle,
			Right,
			Forward,
			Back,
			__COUNT__
		};

		trinex_enum_struct(Button);
		trinex_enum(Button);
	};
}// namespace Engine::Mouse
