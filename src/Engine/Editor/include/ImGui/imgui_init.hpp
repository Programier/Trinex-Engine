#include <Core/export.hpp>

namespace Engine
{
    namespace ImGuiInit
    {
        ENGINE_EXPORT void init(const char* glsl_version = nullptr);
        ENGINE_EXPORT void terminate_imgui();
        ENGINE_EXPORT void render();
        ENGINE_EXPORT void new_frame();
    }
}
