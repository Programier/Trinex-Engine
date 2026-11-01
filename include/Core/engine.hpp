#pragma once

#include <Core/object.hpp>
#include <Input/system.hpp>

#include <Core/engine.generated.hpp>

namespace Trinex
{
	trinex_reflect()
	class trinex_api Engine : public Object
	{
		trinex_reflection_body();

	public:
		enum State : u8
		{
			Created       = 0,
			Initializing  = 1,
			Loading       = 2,
			Starting      = 3,
			Running       = 4,
			ExitRequested = 5,
			Stopping      = 6,
			Stopped       = 7,
			Failed        = 8,
		};

	private:
		u64 m_frame_index;
		f32 m_delta_time;
		f32 m_prev_time;

		State m_state = State::Created;

	private:
		bool transition_to(State state);
		void update();
		void update_viewports();

	public:
		Engine();

		static Engine* instance();
		static i32 launch(int argc, char** argv);
		i32 run();

		virtual void on_initialize();
		virtual void on_load();
		virtual void on_start();
		virtual void on_update(f32 dt);
		virtual void on_stop();
		virtual void on_finalize();

		f32 time_seconds() const;
		Engine& request_exit();

		inline f32 delta_time() const { return m_delta_time; }
		inline u64 frame_index() const { return m_frame_index; }
		inline State state() const { return m_state; }
		inline bool is_initializing() const { return m_state == State::Initializing; }
		inline bool is_loading() const { return m_state == State::Loading; }
		inline bool is_starting() const { return m_state == State::Starting; }
		inline bool is_running() const { return m_state == State::Running; }
		inline bool is_exit_requested() const { return m_state == State::ExitRequested; }
		inline bool is_stopping() const { return m_state == State::Stopping; }
		inline bool is_stopped() const { return m_state == State::Stopped; }
		inline bool is_failed() const { return m_state == State::Failed; }
	};
} // namespace Trinex
