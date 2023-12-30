#pragma once

namespace Engine
{
    void make_dock_window(const char* name, void (*callback)() = nullptr, unsigned int flags = 0);
}
