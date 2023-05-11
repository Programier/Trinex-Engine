#include <Core/engine_types.hpp>
#include <Core/export.hpp>


namespace Engine::ImGuiRenderer
{
    ENGINE_EXPORT void init();
    ENGINE_EXPORT void terminate();
    ENGINE_EXPORT void render();
    ENGINE_EXPORT void new_frame();
}
