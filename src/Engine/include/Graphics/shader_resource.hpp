#pragma once
#include <Core/object.hpp>
#include <Core/resource.hpp>
#include <Core/shader_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT ShaderResource : public Resource<PipelineCreateInfo, Object>
    {
    public:
        bool archive_process(Archive* archive) override;
    };
}
