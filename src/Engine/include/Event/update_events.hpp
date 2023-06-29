#pragma once
#include <Core/export.hpp>


namespace Engine
{
    struct ENGINE_EXPORT UpdateEvents {
        ENGINE_EXPORT static void poll_events();
        ENGINE_EXPORT static void wait_for_event();

    private:
        ENGINE_EXPORT static void process_event();
    };
}// namespace Engine
