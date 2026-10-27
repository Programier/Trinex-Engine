#pragma once

#include <Core/etl/engine_resource.hpp>
#include <Core/deprecated/object.hpp>
#include <Input/system.hpp>

namespace Trinex
{
	class trinex_api Engine : public EngineResource<DeprecatedObject>
	{
		trinex_class(Engine, DeprecatedObject);

		u64 m_frame_index;
		float m_delta_time;
		float m_prev_time;

		u8 m_is_requesting_exit : 1 = false;
		u8 m_is_shutting_down : 1   = false;

	public:
		Engine();

		static Engine* instance();
		static i32 launch(int argc, char** argv);

		virtual i32 init();
		virtual i32 update();
		virtual i32 terminate();
		virtual StringView application_name() const;
		virtual float max_tick_rate() const;
		virtual float gamma() const;

		virtual Engine& request_exit();
		bool is_requesting_exit() const;

		float time_seconds() const;
		bool is_shuting_down() const;

		inline float delta_time() const { return m_delta_time; }
		inline u64 frame_index() const { return m_frame_index; }
	};
} // namespace Trinex
