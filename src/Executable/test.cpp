#include "sol/forward.hpp"
#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/demangle.hpp>
#include <Core/engine_lua.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/deffered_method_invoker.hpp>
#include <Graphics/camera.hpp>
#include <deque>
#include <memory>
#include <type_traits>
#include <vector>


namespace Engine
{
    class NewLuaConsole : public CommandLet
    {
    public:
        void test()
        {
            auto object = Lua::Interpretter::execute_string("return Engine.Camera.create(), Engine.Shader.create();");

            for (auto obj : object) logger->log("Object is camera: %d", (int) obj.is<Camera>());
        }

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
                if (command == "test")
                    test();
                else
                    Lua::Interpretter::execute_string(command);
            }

            return 0;
        }
    };


    register_class(NewLuaConsole, CommandLet);
}// namespace Engine
