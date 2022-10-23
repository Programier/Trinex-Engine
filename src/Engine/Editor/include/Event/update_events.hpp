#pragma once
#include <Core/export.hpp>


namespace Engine::UpdateEvent
{
    ENGINE_EXPORT void poll_events();
    ENGINE_EXPORT void wait_for_event();
}
