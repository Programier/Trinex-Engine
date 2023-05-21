#pragma once
#include <Core/export.hpp>
#include <Core/object.hpp>
#include <Core/transform.hpp>

namespace Engine
{
    class ENGINE_EXPORT Actor : public Object
    {


    public:
        Transform transform;

        virtual Actor& update();
        virtual Actor& load();
        virtual Actor& unload();
        virtual Actor& render();
    };
}
