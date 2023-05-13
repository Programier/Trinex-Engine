#include <Core/engine.hpp>
#include <Core/predef.hpp>
#include <Core/string_functions.hpp>


int main(int argc, char** argv)
{
    Engine::EngineInstance::project_name("TrinexEngineLauncher");
    return trinex_engine_main(argc, argv);
}
