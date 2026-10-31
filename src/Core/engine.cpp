#include <Core/command_line.hpp>
#include <Core/engine.hpp>
#include <Core/entry.hpp>
#include <Core/etl/scoped_value.hpp>
#include <Core/reflection/deprecated/class.hpp>
#include <Platform/platform.hpp>
#include <chrono>

namespace Trinex
{
	static thread_local Engine* s_instance = nullptr;

	static inline std::chrono::high_resolution_clock::time_point current_time_point()
	{
		return std::chrono::high_resolution_clock::now();
	}

	static std::chrono::high_resolution_clock::time_point start_time;

	Engine::Engine()
	{
		start_time    = current_time_point();
		m_frame_index = 0;
		m_prev_time   = 0.f;

		InputSystem::instance()->add_listener(nullptr, EventType::Quit, EventCode::Undefined,
		                                      [this](const InputEvent&) { return request_exit(), true; });
	}

	Engine* Engine::instance()
	{
		return s_instance;
	}

	i32 Engine::launch(int argc, char** argv)
	{
		CommandLine::init(argc, argv);

		// BOOTSTRAP STAGE
		LifeCycle::execute(LifeCycle::Bootstrap);

		// REGISTER STAGE
		LifeCycle::execute(LifeCycle::Register);

		// CONFIGURE STAGE
		LifeCycle::execute(LifeCycle::Configure);

		i32 result = 0;

		if (auto entry = EntryPoint::create())
		{
			result = entry->execute();
		}

		LifeCycle::execute(LifeCycle::Finalize);

		return result;
	}

	i32 Engine::run()
	{
		if (m_state != Created && m_state != ExitRequested)
			return 1;

		ScopedValue<Engine*> current_engine(s_instance, this);

		// INITIALIZE STAGE

		if (transition_to(Initializing))
		{
			on_initialize();
			LifeCycle::execute(LifeCycle::Initialize);
		}

		if (transition_to(Loading))
		{
			on_load();
			LifeCycle::execute(LifeCycle::Load);
		}

		if (transition_to(Starting))
		{
			on_start();
			LifeCycle::execute(LifeCycle::Start);
		}

		if (transition_to(Running))
		{
			while (is_running())
			{
				update();
			}
		}

		transition_to(Stopping);
		LifeCycle::execute(LifeCycle::Stop);
		on_stop();
		LifeCycle::execute(LifeCycle::Shutdown);
		on_finalize();

		transition_to(Stopped);

		return 0;
	}

	bool Engine::transition_to(State state)
	{
		if (m_state == state)
			return true;

		bool is_valid = false;

		switch (m_state)
		{
			case Created: is_valid = state == Initializing || state == ExitRequested; break;
			case Initializing: is_valid = state == Loading || state == ExitRequested; break;
			case Loading: is_valid = state == Starting || state == ExitRequested; break;
			case Starting: is_valid = state == Running || state == ExitRequested; break;
			case Running: is_valid = state == ExitRequested; break;
			case ExitRequested: is_valid = state == Stopping; break;
			case Stopping: is_valid = state == Stopped; break;
			case Stopped:
			case Failed: break;
		}

		if (!is_valid)
			return false;

		m_state = state;
		return true;
	}

	void Engine::update()
	{
		const f32 current_time = time_seconds();
		m_delta_time           = current_time - m_prev_time;
		m_prev_time            = current_time;
		++m_frame_index;

		on_update(m_delta_time);
	}

	void Engine::on_initialize() {}

	void Engine::on_load() {}

	void Engine::on_start() {}

	void Engine::on_update(f32 dt) {}

	void Engine::on_stop() {}

	void Engine::on_finalize() {}

	f32 Engine::time_seconds() const
	{
		return std::chrono::duration_cast<std::chrono::duration<f32>>(current_time_point() - start_time).count();
	}

	Engine& Engine::request_exit()
	{
		transition_to(ExitRequested);
		return *this;
	}
} // namespace Trinex
