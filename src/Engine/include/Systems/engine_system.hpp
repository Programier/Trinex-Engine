#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/system.hpp>

namespace Engine
{
    class ENGINE_EXPORT EngineSystem : public Singletone<EngineSystem, System>
    {
        declare_class(EngineSystem, System);

    private:
        static EngineSystem* _M_instance;

    public:
        friend class Singletone<EngineSystem, System>;
        friend class Object;
    };
}// namespace Engine
