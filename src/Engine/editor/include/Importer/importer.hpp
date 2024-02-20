#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    class Package;
}

namespace Engine::Importer
{
    void import_resource(Package* package, const Path& file);
}
