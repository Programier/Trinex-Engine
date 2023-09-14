#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Event/event_data.hpp>

namespace Engine
{
    static void on_init()
    {
        info_log("TODO", "Implement event data initializer");
    }

    static InitializeController initializer(on_init);
}// namespace Engine
