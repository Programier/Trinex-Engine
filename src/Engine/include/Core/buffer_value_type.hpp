#pragma once
#include <Core/engine_types.hpp>
#include <typeinfo>

namespace Engine
{
    IndexBufferComponent get_type_by_typeid(const std::type_info& info);
}
