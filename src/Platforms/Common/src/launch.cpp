#include <Core/base_engine.hpp>
#include <Core/engine_loop.hpp>
#include <Core/export.hpp>
#include <Core/logger.hpp>
#include <Platform/platform.hpp>

FORCE_ENGINE_EXPORT int main(int argc, const char** argv)
try
{
	Engine::EngineLoop loop;

	loop.init(argc, argv);
	auto engine = Engine::engine_instance;

	while (!engine->is_requesting_exit())
	{
		loop.update();
	}

	loop.terminate();

	return 0;
}
catch (std::exception& e)
{
	error_log("EXCEPTION", "%s", e.what());
	return 1;
}
