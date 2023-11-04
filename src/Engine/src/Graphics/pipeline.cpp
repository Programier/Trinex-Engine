#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/global_uniform_buffer.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>


namespace Engine
{
    Pipeline& Pipeline::rhi_create()
    {
        rhi_destroy();
        _M_rhi_pipeline = engine_instance->rhi()->create_pipeline(this);
        return *this;
    }

    const Pipeline& Pipeline::rhi_bind() const
    {
        if (_M_rhi_pipeline)
        {
            _M_rhi_pipeline->bind();
        }

        GlobalUniformBuffer* ubo = GlobalUniformBuffer::instance();

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

    implement_class(Pipeline, "Engine");
    implement_default_initialize_class(Pipeline);
}// namespace Engine
