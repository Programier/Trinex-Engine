#pragma once
#include <Core/engine_types.hpp>
#include <vector>
#include <Core/export.hpp>


namespace Engine
{
    STRUCT ObjectParameters{
        Point3D position = {0, 0, 0};
        Force force = {0, 0, 0};
        float height = 0;
        float width = 0;
    };

    extern ENGINE_EXPORT float gravity;
}// namespace Engine
