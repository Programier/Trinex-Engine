#include <Core/class.hpp>
#include <Core/base_engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/global_config.hpp>
#include <Core/package.hpp>
#include <Engine/engine_start.hpp>
#include <editor_config.hpp>
#include <Core/engine.hpp>
#include <Window/config.hpp>

class EditorEngine : public Engine::EngineInstance
{
    declare_class(EditorEngine, Engine::EngineInstance);

public:
    Engine::int_t init() override
    {
        Engine::engine_config.config_dir = "resources/configs";

        Engine::global_config.load(Engine::engine_config.config_dir / Engine::Path("editor.json"));
        Engine::editor_config.update().update_using_args();
        Engine::engine_config.update();
        Engine::global_window_config.update();

        return Super::init();
    }
};


implement_class_default_init(EditorEngine, );

// If entry point is not set in engine arguments, we need to set it to Editor
static void on_init()
{
    Engine::Arguments::push_argument(Engine::Arguments::Argument("engine_class", "EditorEngine"), false);
}

static Engine::PreInitializeController initializer(on_init);
