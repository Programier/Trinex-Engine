#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>

namespace Engine
{
	struct Event;

	class ENGINE_EXPORT GameController final
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
		float m_axis_values[Axis::__COUNT__];
		Identifier m_ID;

		GameController(Identifier m_id);

		void axis_motion_listener(const Event& e);
		void controller_removed_listener();

	public:
		CallBacks<void(GameController*)> on_destroy;
		CallBacks<void(GameController*, const Event&)> on_axis_motion;

		Identifier id() const;
		float axis_value(Axis axis, float dead_zone = 0.0f) const;

		static GameController* find(Identifier id);

		friend class GameControllerSystem;
	};
}// namespace Engine
