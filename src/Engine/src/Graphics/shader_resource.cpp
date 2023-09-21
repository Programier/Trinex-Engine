#include <Core/buffer_manager.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Graphics/g_buffer.hpp>
#include <Graphics/shader_resource.hpp>
#include <Window/window.hpp>

namespace Engine
{

#define SHADER_CHECKED_SERIALIZE(a, msg)                                                                               \
    if (!((*archive) & a))                                                                                             \
    {                                                                                                                  \
        error_log("ShaderResource", msg);                                                                              \
        return false;                                                                                                  \
    }

    ENGINE_EXPORT bool operator&(Archive& ar, ShaderUniformBuffer& data)
    {
        ar& data.name;
        ar& data.binding;
        ar& data.size;

        return static_cast<bool>(ar);
    }

    ENGINE_EXPORT bool operator&(Archive& ar, VertexAttribute& data)
    {
        ar& data.name;
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
            error_log("ShaderResource", "Cannot serialize shader resource! No resource found!");
            return false;
        }

        SHADER_CHECKED_SERIALIZE(_M_resources->text.vertex, "Failed to process vertex code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->text.fragment, "Failed to process fragment code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->text.compute, "Failed to process compute code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->text.geometry, "Failed to process geometry code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->binaries.vertex, "Failed to process binary vertex code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->binaries.fragment, "Failed to process binary fragment code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->binaries.compute, "Failed to process binary compute code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->binaries.geometry, "Failed to process binary geometry code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->uniform_buffers, "Failed to process uniform buffer code!");
        SHADER_CHECKED_SERIALIZE(_M_resources->texture_samplers, "Failed to process texture samplers!");
        SHADER_CHECKED_SERIALIZE(_M_resources->shared_buffers, "Failed to process shared buffers!");

        SHADER_CHECKED_SERIALIZE(_M_resources->vertex_info.size, "Failed to process vertex info size!");
        SHADER_CHECKED_SERIALIZE(_M_resources->vertex_info.attributes, "Failed to process vertex info size!");

        byte usage = static_cast<byte>(
                _M_resources->framebuffer == engine_instance->window()->get_rhi_object<RHI::RHI_FrameBuffer>() ? 0 : 1);
        SHADER_CHECKED_SERIALIZE(usage, "Failed to serialize framebuffer usage!");

        if (archive->is_reading())
        {
            if (usage != 0 && GBuffer::instance())
            {
                _M_resources->framebuffer = GBuffer::instance()->get_rhi_object<RHI::RHI_FrameBuffer>();
            }
            else
            {
                _M_resources->framebuffer =
                        EngineInstance::instance()->window()->get_rhi_object<RHI::RHI_FrameBuffer>();
            }
        }

        SHADER_CHECKED_SERIALIZE(_M_resources->state.depth_test, "Failed to serialize depth test state!");
        SHADER_CHECKED_SERIALIZE(_M_resources->state.stencil_test, "Failed to serialize stencil test state!");
        SHADER_CHECKED_SERIALIZE(_M_resources->state.input_assembly, "Failed to serialize input assembly state!");
        SHADER_CHECKED_SERIALIZE(_M_resources->state.rasterizer, "Failed to serialize rasterizer state!");

        SHADER_CHECKED_SERIALIZE(_M_resources->state.color_blending.blend_attachment,
                                 "Failed to serialize color blending (attachments) state!");
        SHADER_CHECKED_SERIALIZE(_M_resources->state.color_blending.logic_op,
                                 "Failed to serialize color blending (logic op) state!");
        SHADER_CHECKED_SERIALIZE(_M_resources->state.color_blending.logic_op,
                                 "Failed to serialize color blending (logic op) state!");
        SHADER_CHECKED_SERIALIZE(_M_resources->state.color_blending.blend_constants.vector,
                                 "Failed to serialize color blending (blend constants) state!");
        SHADER_CHECKED_SERIALIZE(_M_resources->state.color_blending.logic_op_enable,
                                 "Failed to serialize color blending (logic op enable) state!");
        return true;
    }
}// namespace Engine
