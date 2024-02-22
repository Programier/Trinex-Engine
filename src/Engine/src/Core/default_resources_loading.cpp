#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/default_resources_loading.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/package.hpp>
#include <DefaultResources/default.hpp>

namespace Engine
{
    static void load_package(const Vector<byte>& data, const StringView& name)
    {
        VectorReader reader = &data;
        Package* package    = Object::find_package(name, true);
        package->load(&reader);
    }

    ENGINE_EXPORT void load_package_from_memory(const byte* data, size_t size, const StringView& name)
    {
        if (size > 0)
        {
            load_package(Vector<byte>(data, data + size), name);
        }
    }

    void EngineInstance::load_default_resources()
    {
        load_package_from_memory(default_package_data, default_package_len, "DefaultPackage");

        DefaultResourcesInitializeController().execute();
        m_flags(DefaultResourcesInitTriggered, true);
    }
}// namespace Engine
