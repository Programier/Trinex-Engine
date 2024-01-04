#include <Core/class.hpp>
#include <Core/commandlet.hpp>


namespace Engine
{
    class EngineStart : public CommandLet
    {
        declare_class(EngineStart, CommandLet);

    public:
        int_t execute(int_t argc, char** argv) override
        {
            return 0;
        }

        EngineStart& load_configs() override
        {
            return *this;
        }
    };


    implement_engine_class_default_init(EngineStart);
}// namespace Engine
