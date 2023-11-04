#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/system.hpp>

namespace Engine
{
    class ENGINE_EXPORT EngineSystem : public Singletone<EngineSystem, System>
    {
        declare_class(EngineSystem, System);

    private:

    public:
        EngineSystem& create() override;
        EngineSystem& update(float dt) override;

        friend class Object;
    };
}// namespace Engine
