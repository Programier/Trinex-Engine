#include <Core/engine_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT EngineLoop
    {
        int_t preinit(int_t argc, const char** argv);

    public:
        EngineLoop();
        virtual ~EngineLoop();
        int_t init(int_t argc, const char** argv);
        void update();
        void terminate();
    };
}// namespace Engine
