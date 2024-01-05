#pragma once
#include <Core/entry_point.hpp>

namespace Engine
{

    class ENGINE_EXPORT EngineBaseEntryPoint : public EntryPoint
    {
        declare_class(EngineBaseEntryPoint, EntryPoint);

    public:
        EngineBaseEntryPoint();
        EngineBaseEntryPoint& load_configs() override;
    };
}// namespace Engine
