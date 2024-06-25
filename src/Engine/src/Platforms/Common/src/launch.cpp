#include <Core/base_engine.hpp>
#include <Core/engine_loop.hpp>
#include <Core/export.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Platform/platform.hpp>


FORCE_ENGINE_EXPORT int main(int argc, const char** argv)
try
{
    Engine::EngineLoop loop;

    int init_status = loop.init(argc, argv);
    auto engine     = Engine::engine_instance;

    if (init_status == 0)
    {
        while (!engine->is_requesting_exit())
        {
            loop.update();
        }
    }

    loop.terminate();

    return init_status;
}
catch (std::exception& e)
{
    error_log("EXCEPTION", "%s", e.what());
    return 1;
}
