#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>

namespace Engine::FileSystem
{
    ENGINE_EXPORT String dirname_of(const String& filename);
    ENGINE_EXPORT String basename_of(const String& filename);
}
