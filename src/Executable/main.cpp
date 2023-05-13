#include <Core/engine.hpp>
#include <Core/predef.hpp>
#include <Core/string_functions.hpp>


int main(int argc, char** argv)
{
    auto instance = Engine::EngineInstance::instance();
    int status    = instance->start(argc, argv);
    instance->destroy();
    return status;
}
