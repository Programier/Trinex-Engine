#include <Core/buffer_manager.hpp>
#include <Core/logger.hpp>
#include <Graphics/g_buffer.hpp>
#include <Graphics/shader_resource.hpp>

namespace Engine
{

#define SHADER_CHECKED_SERIALIZE(a, msg)                                                                               \
    if (!((*archive) & a))                                                                                             \
    {                                                                                                                  \
        error_log(msg);                                                                                                \
        return false;                                                                                                  \
    }

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
        //ar& data.offset;
        //ar& data.type;

        return static_cast<bool>(ar);
    }

    bool ShaderResource::archive_process(Archive* archive)
    {
        if (!Object::archive_process(archive))
        {
            return false;
        }

        if (archive->is_reading())
        {
            resources(true);
        }

        if (_M_resources == nullptr)
        {
            error_log("ShaderResource: Cannot serialize shader resource! No resource found!");
            return false;
        }

        SHADER_CHECKED_SERIALIZE(_M_resources->text.vertex, "ShaderResource: Failed to process vertex code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->text.fragment, "ShaderResource: Failed to process fragment code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->text.compute, "ShaderResource: Failed to process compute code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->text.geometry, "ShaderResource: Failed to process geometry code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->binaries.vertex,
                                 "ShaderResource: Failed to process binary vertex code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->binaries.fragment,
                                 "ShaderResource: Failed to process binary fragment code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->binaries.compute,
                                 "ShaderResource: Failed to process binary compute code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->binaries.geometry,
                                 "ShaderResource: Failed to process binary geometry code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->uniform_buffers,
                                 "ShaderResource: Failed to process uniform buffer code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->texture_samplers, "ShaderResource: Failed to process texture samplers!");
        SHADER_CHECKED_SERIALIZE(_M_resources->shared_buffers, "ShaderResource: Failed to process shared buffers!");

        SHADER_CHECKED_SERIALIZE(_M_resources->vertex_info.size, "ShaderResource: Failed to process vertex info size!");
        SHADER_CHECKED_SERIALIZE(_M_resources->vertex_info.attributes,
                                 "ShaderResource: Failed to process vertex info size!");

        byte usage = static_cast<byte>(_M_resources->framebuffer_usage == 0 ? 0 : 1);
        SHADER_CHECKED_SERIALIZE(usage, "ShaderResource: Failed to serialize framebuffer usage!");

        if (archive->is_reading())
        {
            if (usage != 0 && GBuffer::instance())
            {
                _M_resources->framebuffer_usage = GBuffer::instance()->id();
            }
            else
            {
                _M_resources->framebuffer_usage = 0;
            }
        }

        SHADER_CHECKED_SERIALIZE(_M_resources->state.depth_test,
                                 "ShaderResource: Failed to serialize depth test state!");
        SHADER_CHECKED_SERIALIZE(_M_resources->state.stencil_test,
                                 "ShaderResource: Failed to serialize stencil test state!");
        SHADER_CHECKED_SERIALIZE(_M_resources->state.input_assembly,
                                 "ShaderResource: Failed to serialize input assembly state!");
        SHADER_CHECKED_SERIALIZE(_M_resources->state.rasterizer,
                                 "ShaderResource: Failed to serialize rasterizer state!");

        SHADER_CHECKED_SERIALIZE(_M_resources->state.color_blending.blend_attachment,
                                 "ShaderResource: Failed to serialize color blending (attachments) state!");
        SHADER_CHECKED_SERIALIZE(_M_resources->state.color_blending.logic_op,
                                 "ShaderResource: Failed to serialize color blending (logic op) state!");
        SHADER_CHECKED_SERIALIZE(_M_resources->state.color_blending.logic_op,
                                 "ShaderResource: Failed to serialize color blending (logic op) state!");
        SHADER_CHECKED_SERIALIZE(_M_resources->state.color_blending.blend_constants.vector,
                                 "ShaderResource: Failed to serialize color blending (blend constants) state!");
        SHADER_CHECKED_SERIALIZE(_M_resources->state.color_blending.logic_op_enable,
                                 "ShaderResource: Failed to serialize color blending (logic op enable) state!");

        SHADER_CHECKED_SERIALIZE(_M_resources->max_textures_binding_per_frame,
                                 "ShaderResource: Failed to serialize max textures binding per frame!");
        return true;
    }
}// namespace Engine
