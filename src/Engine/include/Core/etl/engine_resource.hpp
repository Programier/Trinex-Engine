#pragma once


namespace Engine
{
    template<typename Parent>
    class EngineResource : public Parent
    {
    public:
        using Parent::Parent;

        bool is_engine_resource() const override
        {
            return true;
        }
    };
}// namespace Engine
