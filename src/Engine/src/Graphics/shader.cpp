#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/config.hpp>
#include <Core/engine.hpp>
#include <Core/engine_types.hpp>
#include <Core/logger.hpp>
#include <Core/shader_types.hpp>
#include <Graphics/g_buffer.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/shader.hpp>
#include <api.hpp>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>


namespace Engine
{

    bool operator&(Archive& ar, ShaderUniformBuffer& data)
    {
        ar& data.name;
        ar& data.binding;
        ar& data.size;

        return static_cast<bool>(ar);
    }

    bool operator&(Archive& ar, VertexAtribute& data)
    {
        ar& data.name;
        ar& data.offset;
        ar& data.type;

        return static_cast<bool>(ar);
    }


#define SHADER_CHECKED_SERIALIZE(a, msg)                                                                               \
    if (!((*archive) & a))                                                                                             \
    {                                                                                                                  \
        error_log(msg);                                                                                                \
        return false;                                                                                                  \
    }

    bool ShaderResource::archive_process(Archive* archive)
    {
        if (!mesh_reference.archive_process(archive))
        {
            return false;
        }

        SHADER_CHECKED_SERIALIZE(create_info.text.vertex, "ShaderResource: Failed to process vertex code!");
        SHADER_CHECKED_SERIALIZE(create_info.text.fragment, "ShaderResource: Failed to process fragment code!");
        SHADER_CHECKED_SERIALIZE(create_info.text.compute, "ShaderResource: Failed to process compute code!");
        SHADER_CHECKED_SERIALIZE(create_info.text.geometry, "ShaderResource: Failed to process geometry code!");
        SHADER_CHECKED_SERIALIZE(create_info.binaries.vertex, "ShaderResource: Failed to process binary vertex code!");
        SHADER_CHECKED_SERIALIZE(create_info.binaries.fragment,
                                 "ShaderResource: Failed to process binary fragment code!");
        SHADER_CHECKED_SERIALIZE(create_info.binaries.compute,
                                 "ShaderResource: Failed to process binary compute code!");
        SHADER_CHECKED_SERIALIZE(create_info.binaries.geometry,
                                 "ShaderResource: Failed to process binary geometry code!");
        SHADER_CHECKED_SERIALIZE(create_info.uniform_buffers, "ShaderResource: Failed to process uniform buffer code!");
        SHADER_CHECKED_SERIALIZE(create_info.texture_samplers, "ShaderResource: Failed to process texture samplers!");
        SHADER_CHECKED_SERIALIZE(create_info.shared_buffers, "ShaderResource: Failed to process shared buffers!");

        SHADER_CHECKED_SERIALIZE(create_info.vertex_info.size, "ShaderResource: Failed to process vertex info size!");
        SHADER_CHECKED_SERIALIZE(create_info.vertex_info.attributes,
                                 "ShaderResource: Failed to process vertex info size!");

        byte usage = static_cast<byte>(create_info.framebuffer_usage == 0 ? 0 : 1);
        SHADER_CHECKED_SERIALIZE(usage, "ShaderResource: Failed to serialize framebuffer usage!");

        if (archive->is_reading())
        {
            if (usage != 0 && GBuffer::instance())
            {
                create_info.framebuffer_usage = GBuffer::instance()->id();
            }
            else
            {
                create_info.framebuffer_usage = 0;
            }
        }

        SHADER_CHECKED_SERIALIZE(create_info.state.depth_test, "ShaderResource: Failed to serialize depth test state!");
        SHADER_CHECKED_SERIALIZE(create_info.state.stencil_test,
                                 "ShaderResource: Failed to serialize stencil test state!");
        SHADER_CHECKED_SERIALIZE(create_info.state.input_assembly,
                                 "ShaderResource: Failed to serialize input assembly state!");
        SHADER_CHECKED_SERIALIZE(create_info.state.rasterizer, "ShaderResource: Failed to serialize rasterizer state!");

        SHADER_CHECKED_SERIALIZE(create_info.state.color_blending.blend_attachment,
                                 "ShaderResource: Failed to serialize color blending (attachments) state!");
        SHADER_CHECKED_SERIALIZE(create_info.state.color_blending.logic_op,
                                 "ShaderResource: Failed to serialize color blending (logic op) state!");
        SHADER_CHECKED_SERIALIZE(create_info.state.color_blending.logic_op,
                                 "ShaderResource: Failed to serialize color blending (logic op) state!");
        SHADER_CHECKED_SERIALIZE(create_info.state.color_blending.blend_constants.vector,
                                 "ShaderResource: Failed to serialize color blending (blend constants) state!");
        SHADER_CHECKED_SERIALIZE(create_info.state.color_blending.logic_op_enable,
                                 "ShaderResource: Failed to serialize color blending (logic op enable) state!");

        SHADER_CHECKED_SERIALIZE(create_info.max_textures_binding_per_frame,
                                 "ShaderResource: Failed to serialize max textures binding per frame!");
        return true;
    }


    static const Map<std::type_index, typeof(ShaderDataType::Bool)> types = {
            {std::type_index(typeid(bool)), ShaderDataType::Bool},
            {std::type_index(typeid(int_t)), ShaderDataType::Int},
            {std::type_index(typeid(uint_t)), ShaderDataType::UInt},
            {std::type_index(typeid(float)), ShaderDataType::Float},
            {std::type_index(typeid(Vector2D)), ShaderDataType::Vec2},
            {std::type_index(typeid(Vector3D)), ShaderDataType::Vec3},
            {std::type_index(typeid(Vector4D)), ShaderDataType::Vec4},
            {std::type_index(typeid(IntVector2D)), ShaderDataType::IVec2},
            {std::type_index(typeid(IntVector3D)), ShaderDataType::IVec3},
            {std::type_index(typeid(IntVector4D)), ShaderDataType::IVec4},
            {std::type_index(typeid(UIntVector2D)), ShaderDataType::UVec2},
            {std::type_index(typeid(UIntVector3D)), ShaderDataType::UVec3},
            {std::type_index(typeid(UIntVector4D)), ShaderDataType::UVec4},
            {std::type_index(typeid(BoolVector2D)), ShaderDataType::BVec2},
            {std::type_index(typeid(BoolVector3D)), ShaderDataType::BVec3},
            {std::type_index(typeid(BoolVector4D)), ShaderDataType::BVec4},
            {std::type_index(typeid(Matrix2f)), ShaderDataType::Mat2},
            {std::type_index(typeid(Matrix3f)), ShaderDataType::Mat3},
            {std::type_index(typeid(Matrix4f)), ShaderDataType::Mat4},
    };

    ENGINE_EXPORT void ShaderDataType::private_type_of(ShaderDataType& result, const std::type_index& index)
    {
        result.type = types.find(index)->second;
    }

    register_class(Engine::Shader, Engine::ApiObject);

    Shader::Shader()
    {}


    Shader::Shader(const PipelineCreateInfo& params)
    {
        load(params);
    }


    Shader& Shader::load(const PipelineCreateInfo& params)
    {
        destroy();
        EngineInstance::instance()->api_interface()->create_shader(_M_ID, params);
        return *this;
    }

    const Shader& Shader::use() const
    {
        EngineInstance::instance()->api_interface()->use_shader(_M_ID);
        return *this;
    }

    void Shader::unbind()
    {
        EngineInstance::instance()->api_interface()->use_shader(0);
    }


    static const Map<String, ShaderDataType> semantic_types = {
            {"position", ShaderDataType::type_of<Vector2D>()},
            {"tex_coord", ShaderDataType::type_of<Vector2D>()},
            {"color", ShaderDataType::type_of<Vector4D>()},
            {"normal", ShaderDataType::type_of<Vector3D>()},
            {"tangent", ShaderDataType::type_of<Vector3D>()},
            {"binormal", ShaderDataType::type_of<Vector3D>()},
            {"blend_weight", ShaderDataType::type_of<Vector4D>()},
            {"blend_indices", ShaderDataType::type_of<IntVector4D>()},
    };

    static const Map<String, VertexBufferSemantic> semantic_names = {
            {"position", VertexBufferSemantic::Position},
            {"tex_coord", VertexBufferSemantic::TexCoord},
            {"color", VertexBufferSemantic::Color},
            {"normal", VertexBufferSemantic::Normal},
            {"tangent", VertexBufferSemantic::Tangent},
            {"binormal", VertexBufferSemantic::Binormal},
            {"blend_weight", VertexBufferSemantic::BlendWeight},
            {"blend_indices", VertexBufferSemantic::BlendIndices},
    };

    static bool find_type(const String& semantic_name, ShaderDataType& type, byte& index,
                          VertexBufferSemantic& semantic)
    {
        if (!semantic_name.starts_with("in_"))
        {
            return false;
        }

        auto it = semantic_name.find_last_of("_");

        if (it == String::npos || it < 3)
        {
            return false;
        }


        String name      = semantic_name.substr(3, it - 3);
        auto semantic_it = semantic_types.find(name);
        if (semantic_it == semantic_types.end())
            return false;
        semantic = semantic_names.find(name)->second;
        type     = semantic_it->second;

        name  = semantic_name.substr(it + 1, semantic_name.length() - it - 1);
        index = static_cast<byte>(std::stoi(name));
        return true;
    }

    Shader& Shader::load_to_gpu()
    {
        ShaderResource* resource = resources();
        StaticMesh* mesh         = resource->mesh_reference.instance()->instance_cast<StaticMesh>();

        if (mesh != nullptr)
        {
            VertexBufferInfo& vertex_info   = resource->create_info.vertex_info;
            const StaticMeshSemanticInfo& a = mesh->semantic_info();
            vertex_info.size                = a.vertex_size();

            for (auto& entry : vertex_info.attributes)
            {
                byte index;
                VertexBufferSemantic semantic;
                if (!find_type(entry.name, entry.type, index, semantic))
                    continue;
                entry.offset = mesh->semantic_info().semantic_offset(semantic, index);
            }
        }

        load(resource->create_info);
        return *this;
    }

    bool Shader::archive_process(Archive* archive)
    {
        if (!ApiObject::archive_process(archive))
        {
            return false;
        }

        if (archive->is_reading())
        {
            resources(true);
        }

        if (!_M_resources->archive_process(archive))
        {
            return false;
        }

        if (archive->is_reading())
        {
            if (engine_instance->api() != EngineAPI::NoAPI && engine_config.load_shaders_to_gpu)
            {
                load_to_gpu();
            }

            if (engine_config.delete_resources_after_load)
            {
                delete_resources();
            }
        }

        return static_cast<bool>(archive);
    }


}// namespace Engine
