#include <Core/engine_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT EngineLoop
    {
    public:
        EngineLoop();
        virtual ~EngineLoop();

        virtual int_t preinit(int_t argc, const char** argv);
        virtual int_t init();
        virtual void update();
        virtual void terminate();
    };
}// namespace Engine
