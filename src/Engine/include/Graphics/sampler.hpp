#pragma once
#include <Core/api_object.hpp>
#include <Core/resource.hpp>
#include <Core/rhi_initializers.hpp>

namespace Engine
{


    class ENGINE_EXPORT Sampler : public ApiBindingObject
    {
        declare_class(Sampler, ApiObject);

    public:
        SamplerCreateInfo info;

        Sampler& create();
        bool archive_process(Archive* archive) override;
    };
}// namespace Engine