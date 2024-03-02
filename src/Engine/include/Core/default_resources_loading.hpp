#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    ENGINE_EXPORT class Object* load_object_from_memory(const byte* data, size_t size, const StringView& name,
                                                        class Package* package = nullptr);
}
