#pragma once
#include <Graphics/scene.hpp>

namespace Editor
{
    struct Resources
    {
        static Engine::Scene scene;
        static Engine::ObjectInstance* object_for_rendering;
        static Engine::ObjectInstance* object_for_properties;
    };
}
