#include <Core/engine.hpp>


int main(int argc, char** argv)
{
    auto instance = Engine::EngineInstance::instance();
    int status    = instance->start(argc, argv);
    instance->destroy();
    return status;
}
