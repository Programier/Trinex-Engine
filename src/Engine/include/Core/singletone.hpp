#pragma once
#include <Core/object.hpp>

namespace Engine
{
    class ENGINE_EXPORT Singletone : public Object
    {
    public:
        Singletone* initialize_singletone_instance()
    };
}
