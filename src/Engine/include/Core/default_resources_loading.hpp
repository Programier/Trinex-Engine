#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    ENGINE_EXPORT void load_package_from_memory(const byte* data, size_t size, const StringView& name);
}
