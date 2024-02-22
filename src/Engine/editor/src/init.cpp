#include "DefaultResources/editor.hpp"
#include <Core/default_resources_loading.hpp>
#include <Core/engine_loading_controllers.hpp>


namespace Engine
{

    static void resource_loading()
    {
        load_package_from_memory(editor_package_data, editor_package_len, "Editor");
    }

    static DefaultResourcesInitializeController on_init(resource_loading, "Load Editor Package");
}// namespace Engine
