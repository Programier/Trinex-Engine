#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/engine_lua.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/deffered_method_invoker.hpp>


namespace Engine
{

    class LuaConsole : public CommandLet
    {
    public:
        using Super = CommandLet;

        virtual int_t execute(int_t argc, char** argv) override
        {
            String command;
            auto get_command = [&command]() {
                std::clog << "--> ";
                std::getline(std::cin, command);
                if (command == "exit")
                    return false;

                return true;
            };

            while (get_command())
            {
                Lua::Interpretter::execute_string(command);
            }

            return 0;
        }
    };


    static void on_init()
    {
        register_class(LuaConsole);
    }

    static InitializeController initializer(on_init);
}// namespace Engine
