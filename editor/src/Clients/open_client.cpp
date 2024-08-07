#include <Clients/open_client.hpp>
#include <Window/config.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
    void open_material_editor()
    {
        WindowConfig new_config;
        new_config.client       = "Engine::MaterialEditorClient";
        WindowManager::instance()->create_window(new_config);
    }

    void open_editor()
    {
        WindowConfig new_config;
        new_config.client       = "Engine::EditorClient";
        WindowManager::instance()->create_window(new_config);
    }

    void open_script_debugger()
    {
        WindowConfig new_config;
        new_config.client       = "Engine::ScriptDebuggerClient";
        WindowManager::instance()->create_window(new_config);
    }
}// namespace Engine
