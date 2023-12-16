#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/engine_config.hpp>
#include <Core/file_manager.hpp>
#include <Core/global_config.hpp>
#include <Core/logger.hpp>

namespace Engine
{
    int_t CommandLet::execute(int_t argc, char** argv)
    {
        info_log("CommandLet",
                 "You must override method 'int execute(int argc, char** argv)' for using your commandlet!");
        return 0;
    }

    CommandLet& CommandLet::load_configs()
    {
        return *this;
    }

    implement_class(CommandLet, "Engine", 0);
    implement_default_initialize_class(CommandLet);
}// namespace Engine
