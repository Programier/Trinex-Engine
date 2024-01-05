#include <Core/class.hpp>
#include <Core/entry_point.hpp>
#include <Core/engine_config.hpp>
#include <Core/file_manager.hpp>
#include <Core/global_config.hpp>
#include <Core/logger.hpp>

namespace Engine
{
    int_t EntryPoint::execute(int_t argc, char** argv)
    {
        info_log("EntryPoint",
                 "You must override method 'int execute(int argc, char** argv)' for using your EntryPoint!");
        return 0;
    }

    EntryPoint& EntryPoint::load_configs()
    {
        return *this;
    }

    implement_class(EntryPoint, "Engine", 0);
    implement_default_initialize_class(EntryPoint);
}// namespace Engine
