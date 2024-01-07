#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/global_config.hpp>
#include <Core/package.hpp>
#include <Core/engine_config.hpp>
#include <Engine/engine_start.hpp>

class Editor : public Engine::EngineBaseEntryPoint
{
    declare_class(Editor, Engine::EngineBaseEntryPoint);

public:
    Engine::int_t execute(Engine::int_t argc, char** argv) override
    {
        Engine::Package::load_package("Editor");
        Engine::engine_instance->launch();
        return 0;
    }

    Editor& load_configs() override
    {
        Super::load_configs();
        Engine::global_config.load(Engine::engine_config.config_dir / Engine::Path("editor.json"));
        return *this;
    }
};


implement_class_default_init(Editor, );

// If entry point is not set in engine arguments, we need to set it to Editor
static void on_init()
{
    Engine::engine_instance->args().push_argument(Engine::Arguments::Argument("entry", "Editor"), false);
}

static Engine::InitializeController initializer(on_init);
