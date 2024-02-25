#include "DefaultResources/editor.hpp"
#include <Core/default_resources_loading.hpp>
#include <Core/engine_loading_controllers.hpp>


namespace Engine
{
    namespace Icons
    {
        extern void on_editor_package_loaded();
    }

    static void resource_loading()
    {
        load_package_from_memory(editor_package_data, editor_package_len, "Editor");
        Icons::on_editor_package_loaded();
    }

    static DefaultResourcesInitializeController on_init(resource_loading, "Load Editor Package");
}// namespace Engine
