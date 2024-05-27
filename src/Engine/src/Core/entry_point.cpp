#include <Core/class.hpp>
#include <Core/base_engine.hpp>
#include <Core/entry_point.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>

namespace Engine
{
    void EntryPoint::init(int_t argc, char** argv)
    {}

    void EntryPoint::tick()
    {
        info_log("EntryPoint", "You must override method 'void tick()' for using your EntryPoint!");
        engine_instance->request_exit();
    }

    void EntryPoint::terminate()
    {}

    implement_class(EntryPoint, Engine, 0);
    implement_default_initialize_class(EntryPoint);
}// namespace Engine
