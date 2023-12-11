#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/constants.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/global_config.hpp>
#include <viewport_client.hpp>

class Editor : public Engine::CommandLet
{
    declare_class(Editor, Engine::CommandLet);

public:
    Engine::int_t execute(Engine::int_t argc, char** argv) override
    {
        Engine::engine_instance->launch();
        return 0;
    }

    CommandLet& load_configs() override
    {
        Engine::global_config.load(Engine::Constants::configs_dir / Engine::Path("editor.json"));
        Engine::engine_config.update();
        return *this;
    }
};


implement_class_default_init(Editor, "");
