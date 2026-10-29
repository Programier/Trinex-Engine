#include <Core/engine.hpp>
#include <Core/reflection/deprecated/class.hpp>
#include <Platform/platform.hpp>
#include <chrono>

namespace Trinex
{
	trinex_implement_engine_class_default_init(Engine, 0);

	static Engine* s_instance = nullptr;

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

		flags.set(Flags::StandAlone);
		flags.remove(Flags::IsAvailableForGC);

		InputSystem::instance()->add_listener(nullptr, EventType::Quit, EventCode::Undefined,
		                                      [this](const InputEvent&) { return request_exit(), true; });
	}

	Engine* Engine::instance()
	{
		return s_instance;
	}

	i32 Engine::launch(int argc, char** argv)
	{
		// 		Bootstrap  = -9,
		//         Register   = -8,
		//         Configure  = -7,
		//         Initialize = -6,
		//         Load       = -5,
		//         Start      = -4,

		//         // Shutdown
		//         Stop     = -3,
		//         Shutdown = -2,
		//         Finalize = -1

		// BOOTSTRAP STAGE
		LifeCycle::execute(LifeCycle::Bootstrap);

		// REGISTER STAGE
		LifeCycle::execute(LifeCycle::Register);
		
		LifeCycle::execute(LifeCycle::Configure);

		while (true)
		{
			Platform::Context::instance()->update();
		}

		return 0;
	}

	i32 Engine::init()
	{
		return 0;
	}

	i32 Engine::update()
	{

		return 0;
	}

	i32 Engine::terminate()
	{
		m_is_shutting_down = true;
		return 0;
	}

	StringView Engine::application_name() const
	{
		return "Trinex Example";
	}

	float Engine::max_tick_rate() const
	{
		return 60.f;
	}

	float Engine::gamma() const
	{
		return 1.f;
	}

	Engine& Engine::request_exit()
	{
		m_is_requesting_exit = true;
		return *this;
	}

	bool Engine::is_requesting_exit() const
	{
		return m_is_requesting_exit;
	}

	bool Engine::is_shuting_down() const
	{
		return m_is_shutting_down;
	}

	float Engine::time_seconds() const
	{
		return std::chrono::duration_cast<std::chrono::duration<float>>(current_time_point() - start_time).count();
	}
} // namespace Trinex
