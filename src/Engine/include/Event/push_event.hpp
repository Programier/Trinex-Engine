#pragma once
#include <Core/export.hpp>

namespace Engine::PushEvent
{
    ENGINE_EXPORT void on_terminate_event();
    ENGINE_EXPORT void on_quit_event();
    ENGINE_EXPORT void on_resume_event();
    ENGINE_EXPORT void on_pause_event();
    ENGINE_EXPORT void on_low_memory();
}
