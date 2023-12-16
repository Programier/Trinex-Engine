#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_target_base.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>

namespace Engine
{
    Pipeline& Pipeline::rhi_create()
    {
        _M_rhi_object.reset(engine_instance->rhi()->create_pipeline(this));
        return *this;
    }

    const Pipeline& Pipeline::rhi_bind() const
    {
        if (_M_rhi_object)
        {
            rhi_object<RHI_Pipeline>()->bind();
        }

        UniformBuffer* ubo = RenderTargetBase::current_target()->uniform_buffer();

        if (vertex_shader.ptr()->global_ubo_location.is_valid())
        {
            ubo->rhi_bind(vertex_shader.ptr()->global_ubo_location);
        }

        if (fragment_shader.ptr()->global_ubo_location.is_valid())
        {
            ubo->rhi_bind(fragment_shader.ptr()->global_ubo_location);
        }

        return *this;
    }

    implement_class(Pipeline, "Engine", 0);
    implement_default_initialize_class(Pipeline);
}// namespace Engine
