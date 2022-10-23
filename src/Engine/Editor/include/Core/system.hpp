#pragma once
#include <Core/export.hpp>

namespace Engine
{
    enum class SystemName
    {
        LINUX_OS,
        WINDOWS_OS,
        ANDROID_OS
    };


    extern const ENGINE_EXPORT SystemName system_name;
}
