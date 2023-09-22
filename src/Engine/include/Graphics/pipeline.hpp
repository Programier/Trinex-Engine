#pragma once
#include <Core/api_object.hpp>

namespace Engine
{
    class ENGINE_EXPORT Pipeline : public ApiObject
    {
        declare_class(Pipeline, ApiObject);

    public:

        Pipeline& rhi_create() override;
    };
}// namespace Engine
