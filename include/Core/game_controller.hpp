#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
	class ENGINE_EXPORT GameController
	{
	public:
		enum Axis : EnumerateType
		{
			Unknown = 0,
			LeftX,
			LeftY,
			RightX,
			RightY,
			TriggerLeft,
			TriggerRight,
			__COUNT__
		};

	private:
		short_t m_axis_values[Axis::__COUNT__];
		Identifier m_ID;

		GameController(Identifier m_id);

	public:
		Identifier id() const;
		short_t axis_value(Axis axis) const;
		float axis_value_normalized(Axis axis) const;

		~GameController();
		friend class GameControllerSystem;
	};
}// namespace Engine
