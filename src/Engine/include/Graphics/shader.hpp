#pragma once

#include <Core/api_object.hpp>
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/object_ref.hpp>
#include <Core/resource.hpp>


namespace Engine
{
    struct PipelineCreateInfo;

    class ENGINE_EXPORT Shader : public ApiObject
    {
        declare_class(Shader, ApiObject);

    public:
        delete_copy_constructors(Shader);
        constructor_hpp(Shader);
        Shader(const PipelineCreateInfo& params);

        Shader& load(const PipelineCreateInfo& params);

        const Shader& use() const;
        static void unbind();
    };

}// namespace Engine
