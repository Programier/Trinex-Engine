#include <Core/engine_types.hpp>
#include <imgui.h>

namespace Engine::ImGuiRenderer
{
    class ENGINE_EXPORT DrawData final
    {
        ImDrawData _M_draw_data;

    public:
        ImDrawData* draw_data();
        DrawData& release();
        DrawData& copy(ImDrawData* draw_data);

        ~DrawData();
    };
}// namespace Engine::ImGuiRenderer
