#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <DefaultResources/default.hpp>
#include <Core/package.hpp>

namespace Engine
{
    static void load_package(const Vector<byte>& data, const StringView& name)
    {
        VectorReader reader = &data;
        Package* package = Object::find_package(name, true);
        package->load(&reader);
    }

    void EngineInstance::load_default_resources()
    {
        if (default_package_len != 0)
        {
           // load_package(Vector<byte>(default_package_data, default_package_data + default_package_len), "Default");
        }

        DefaultResourcesInitializeController().execute();
        _M_flags(DefaultResourcesInitTriggered, true);
    }
}// namespace Engine
