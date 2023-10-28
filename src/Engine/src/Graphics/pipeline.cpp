#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>


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
        return *this;
    }

    implement_class(Pipeline, "Engine");
    implement_default_initialize_class(Pipeline);
}// namespace Engine
