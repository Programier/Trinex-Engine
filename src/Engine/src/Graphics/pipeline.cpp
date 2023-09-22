#include <Core/engine.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>


namespace Engine
{
    Pipeline& Pipeline::rhi_create()
    {
        Super::rhi_create();
        _M_rhi_pipeline = engine_instance->api_interface()->create_pipeline(this);
        return *this;
    }
}// namespace Engine
