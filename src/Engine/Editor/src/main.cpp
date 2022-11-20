#include <Core/logger.hpp>
#include <Graphics/hitbox.hpp>
#include <editor.hpp>


int main()
try
{
    delete &(*new Editor::Application()).loop();
    return 0;
}
catch (const std::exception& e)
{
    Engine::logger->log(e.what());
    return 1;
}
