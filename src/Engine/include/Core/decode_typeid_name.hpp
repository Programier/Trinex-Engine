#pragma once
#include <Core/engine_types.hpp>
#include <typeinfo>
#include <Core/export.hpp>

namespace Engine
{
    ENGINE_EXPORT String decode_name(const std::type_info& info);
}
