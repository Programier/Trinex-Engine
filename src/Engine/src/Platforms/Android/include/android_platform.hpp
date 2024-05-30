#pragma once
#include <Platform/platform.hpp>


struct android_app;

namespace Engine::Platform
{
    void initialize_android_application(struct android_app* app);
    android_app* android_application();
}
