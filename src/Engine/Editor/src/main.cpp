#include <Core/logger.hpp>
#include <editor.hpp>

int main()
try
{
    Engine::Editor editor;
    editor.loop();
    return 0;
}
catch (const std::exception& e)
{
    Engine::logger->log(e.what());
    return 1;
}
