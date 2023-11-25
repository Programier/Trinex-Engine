#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/executable_object.hpp>
#include <Core/system.hpp>

namespace Engine
{
    class ENGINE_EXPORT LogicSystem : public Singletone<LogicSystem, System>
    {
        declare_class(LogicSystem, System);

    public:
        LogicSystem& create() override;
        LogicSystem& update(float dt) override;
        LogicSystem& shutdown() override;
        LogicSystem& wait() override;
        class Class* depends_on() const override;
        friend class Object;
    };
}// namespace Engine
