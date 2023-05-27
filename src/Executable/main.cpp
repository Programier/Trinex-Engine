#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>

static void preinit()
{
    Engine::EngineInstance::project_name("TrinexEngineLauncher");
}

static Engine::PreInitializeController controller(preinit);
