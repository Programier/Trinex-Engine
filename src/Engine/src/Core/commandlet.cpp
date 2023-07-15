#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/logger.hpp>

namespace Engine
{
    int_t CommandLet::execute(int_t argc, char** argv)
    {
        info_log("CommandLet",
                 "You must override method 'int execute(int argc, char** argv)' for using your commandlet!");
        return 0;
    }

    void CommandLet::on_config_load()
    {}

    register_class(Engine::CommandLet);
}// namespace Engine
