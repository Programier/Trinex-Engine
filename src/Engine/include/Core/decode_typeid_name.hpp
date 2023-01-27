#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <typeinfo>

namespace Engine
{
    ENGINE_EXPORT String decode_name(const std::type_info& info);
    ENGINE_EXPORT String decode_name(const String& name);
}// namespace Engine
