#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_target.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_engine_class_default_init(RenderTarget, 0);

    RenderTarget::RenderTarget() : size({0, 0})
    {}

    RenderTarget& RenderTarget::rhi_create()
    {
        Super::rhi_create();
        m_rhi_object.reset(rhi->create_render_target(this));
        return *this;
    }

    Size2D RenderTarget::render_target_size() const
    {
        return size;
    }

    RenderTarget::~RenderTarget()
    {}
}// namespace Engine
