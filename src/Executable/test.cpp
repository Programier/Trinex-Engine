#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/engine_lua.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/deffered_method_invoker.hpp>
#include <Graphics/camera.hpp>


namespace Engine
{

    class NewLuaConsole : public CommandLet
    {
    public:
        void test()
        {
            const char* code = "camera = Engine.Camera.create();";
            LuaInterpretter::execute_string(code);

            luabridge::LuaRef camera = LuaInterpretter::execute_string("return camera;")[0];
            logger->log("LUA object is instance of camera: %d", (int) camera.isInstance<Camera>());
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
                    LuaInterpretter::execute_string(command);
            }

            return 0;
        }
    };


    register_class(NewLuaConsole, CommandLet);
}// namespace Engine
