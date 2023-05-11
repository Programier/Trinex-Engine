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
        virtual int execute(int argc, char** argv) override
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
                LuaInterpretter::execute_string(command);
            }

            return 0;
        }
    };


    register_class(LuaConsole, CommandLet);
}// namespace Engine
