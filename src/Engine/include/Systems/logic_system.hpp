#pragma once
#include <Core/etl/singletone.hpp>
#include <Core/executable_object.hpp>
#include <Core/system.hpp>

namespace Engine
{
    class ENGINE_EXPORT LogicSystem : public Singletone<LogicSystem, System>
    {
        declare_class(LogicSystem, System);

    private:
        int_t private_update();

    public:
        LogicSystem& create() override;
        LogicSystem& update(float dt) override;
        LogicSystem& shutdown() override;
        LogicSystem& wait() override;
        friend class Object;
    };
}// namespace Engine
