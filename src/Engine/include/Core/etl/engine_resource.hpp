#pragma once


namespace Engine
{
    template<typename Parent>
    class EngineResource : public Parent
    {
    public:
        bool is_engine_resource() const
        {
            return true;
        }
    };
}// namespace Engine
