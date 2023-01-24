#pragma once
#include <Core/export.hpp>
#include <Core/object.hpp>

namespace Engine
{
    class SceneInterface : public Object
    {
    public:
        virtual SceneInterface& tick(float dt) = 0;
        virtual SceneInterface& render() = 0;
    };
}// namespace Engine
