#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    class Object;

    Object* load_gltf_object(const String& filename);
}
