#include <Core/export.hpp>

namespace Engine
{
    class Event;

    namespace ImGuiWindowBackend
    {
        ENGINE_EXPORT void on_event_recieved(const Event& event);
        ENGINE_EXPORT void disable_events();
        ENGINE_EXPORT void enable_events();
    }
}// namespace Engine
