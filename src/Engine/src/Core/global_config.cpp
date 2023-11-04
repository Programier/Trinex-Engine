#include <Core/global_config.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/file_manager.hpp>

namespace Engine
{
    ENGINE_EXPORT JSON::Object global_config;

    static void initialize()
    {
        global_config.load(FileManager::root_file_manager()->work_dir() / Path("configs/config.json"));
    }

    static InitializeController on_init(initialize);
}
