#pragma once
#include <BasicFunctional/engine_types.hpp>
#include <vector>


namespace Engine
{
    struct ObjectParameters{
        Point3D position = {0, 0, 0};
        Force force = {0, 0, 0};
        float height = 0;
        float width = 0;
    };

    extern float gravity;
}// namespace Engine
