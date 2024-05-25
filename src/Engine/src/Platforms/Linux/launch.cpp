#include <Core/base_engine.hpp>
#include <Core/engine_loop.hpp>
#include <Core/export.hpp>

FORCE_ENGINE_EXPORT int main(int argc, const char** argv)
try
{
    Engine::EngineLoop loop;
    loop.preinit(argc, argv);
    loop.init();

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
    printf("Exception: %s\n", e.what());
    return 1;
}
