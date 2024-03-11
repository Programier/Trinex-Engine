#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/render_thread.hpp>
#include <Graphics/render_target.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(RenderTarget, Engine, 0);
    implement_default_initialize_class(RenderTarget);


    RenderTarget::RenderTarget() : size({0, 0})
    {}

    RenderTarget& RenderTarget::rhi_create()
    {
        Super::rhi_create();
        m_rhi_object.reset(EngineInstance::instance()->rhi()->create_render_target(this));
        return *this;
    }

    Size2D RenderTarget::render_target_size() const
    {
        return size;
    }

    RenderTarget::~RenderTarget()
    {}
}// namespace Engine
