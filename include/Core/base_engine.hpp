#pragma once

#include <Core/etl/engine_resource.hpp>
#include <Core/object.hpp>

namespace Trinex
{
	class ENGINE_EXPORT BaseEngine : public EngineResource<Object>
	{
		trinex_class(BaseEngine, Object);

		u64 m_frame_index;
		float m_delta_time;
		float m_prev_time;

		u8 m_is_requesting_exit : 1 = false;
		u8 m_is_shutting_down : 1   = false;

	public:
		BaseEngine();

		virtual i32 init();
		virtual i32 update();
		virtual i32 terminate();
		virtual StringView application_name() const;
		virtual float max_tick_rate() const;
		virtual float gamma() const;

		virtual BaseEngine& request_exit();

		bool is_requesting_exit() const;

		float time_seconds() const;
		bool is_shuting_down() const;

		inline float delta_time() const { return m_delta_time; }
		inline u64 frame_index() const { return m_frame_index; }
	};

	ENGINE_EXPORT extern BaseEngine* engine_instance;
}// namespace Trinex
