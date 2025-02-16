#pragma once
#include <Core/engine_types.hpp>

namespace Engine::Mouse
{
	enum Status : EnumerateType
	{
		Released = 0,
		JustReleased,
		JustPressed,
		Pressed,
	};

	enum Button : EnumerateType
	{
		Unknown,
		Left,
		Middle,
		Right,
		Forward,
		Back,
		__COUNT__
	};
}// namespace Engine::Mouse
