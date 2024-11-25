#include <Core/arguments.hpp>
#include <Core/base_engine.hpp>
#include <Core/engine_loop.hpp>
#include <Core/export.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <android_native_app_glue.h>
#include <android_platform.hpp>

FORCE_ENGINE_EXPORT extern "C" int trinex_engine_android_main(int argc, const char** argv)
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
	error_log("Engine", "Exception: %s\n", e.what());
	return 1;
}

// This method will be called from NativeActivity
FORCE_ENGINE_EXPORT extern "C" void android_main(struct android_app* app)
{
	Engine::Platform::initialize_android_application(app);
	trinex_engine_android_main(0, nullptr);
}
