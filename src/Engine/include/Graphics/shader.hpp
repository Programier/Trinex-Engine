#pragma once

#include <Core/api_object.hpp>
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/implement.hpp>
#include <Core/resource.hpp>
#include <Core/shader_types.hpp>
#include <string>
#include <Core/object_ref.hpp>

namespace Engine
{

    class StaticMesh;

    struct ShaderResource : SerializableObject {
        PipelineCreateInfo create_info;
        ObjectReference<class StaticMesh> mesh_reference;

        bool archive_process(Archive* archive) override;
    };

    class ENGINE_EXPORT Shader : public Resource<ShaderResource, ApiObject>
    {
    private:
        Shader& load_to_gpu();

    public:
        delete_copy_constructors(Shader);
        constructor_hpp(Shader);
        Shader(const PipelineCreateInfo& params);

        Shader& load(const PipelineCreateInfo& params);

        const Shader& use() const;
        static void unbind();

        bool archive_process(Archive* archive) override;
    };

}// namespace Engine
