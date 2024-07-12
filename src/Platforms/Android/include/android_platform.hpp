#pragma once
#include <Core/enums.hpp>
#include <Platform/platform.hpp>


struct android_app;

namespace Engine::Platform
{
    struct AndroidPlatformInfo {
        String app_package_name;
        String device_manufacturer;
        String device_model;
        String device_build_number;
        String system_version;
        String system_language;
        String cache_dir;
        String executable_path;
        String libraries_path;
        uint_t screen_width;
        uint_t screen_height;
        Orientation orientation;
        bool mouse_in_relative_mode = false;
    };

    extern AndroidPlatformInfo m_android_platform_info;

    void initialize_android_application(struct android_app* app);
    android_app* android_application();
}// namespace Engine::Platform
