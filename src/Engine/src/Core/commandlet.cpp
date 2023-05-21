#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/logger.hpp>

namespace Engine
{
    int CommandLet::execute(int argc, char** argv)
    {
        info_log("CommandLet: You must override method 'int execute(int argc, char** argv)' for usin your commandlet!");
        return 0;
    }

    void CommandLet::on_config_load()
    {}

    register_class(Engine::CommandLet, Engine::Object);
}// namespace Engine
