#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/global_config.hpp>
#include <Engine/engine_start.hpp>

namespace Engine
{
    EngineBaseEntryPoint::EngineBaseEntryPoint()
    {}

    EngineBaseEntryPoint& EngineBaseEntryPoint::load_configs()
    {
        global_config.load(engine_config.config_dir / "engine.json");
        return *this;
    }

    class EngineStart : public EngineBaseEntryPoint
    {
        declare_class(EngineStart, EngineBaseEntryPoint);

    public:
        int_t execute(int_t argc, char** argv) override
        {
            return engine_instance->launch();
        }
    };


    implement_engine_class_default_init(EngineBaseEntryPoint);
    implement_engine_class_default_init(EngineStart);
}// namespace Engine
