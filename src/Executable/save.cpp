#include <Core/package.hpp>
#include <Graphics/sampler.hpp>

namespace Engine
{
    void save_package(Package* package)
    {
        package->save();
    }
}// namespace Engine
