#pragma once

#include <Core/api_object.hpp>
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/implement.hpp>
#include <string>
#include <Core/shader_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT Shader : public ApiObject
    {
    public:
        delete_copy_constructors(Shader);
        constructor_hpp(Shader);
        Shader(const PipelineCreateInfo& params);

        Shader& load(const PipelineCreateInfo& params);

        const Shader& use() const;
        static void unbind();
    };

}// namespace Engine
