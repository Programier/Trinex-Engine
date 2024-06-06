#include <Core/class.hpp>
#include <Core/base_engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/threading.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_target_base.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_engine_class_default_init(RenderTargetBase, 0);

    RenderTargetBase* RenderTargetBase::m_current_target = nullptr;

    RenderTargetBase::RenderTargetBase()
    {}

    RenderTargetBase& RenderTargetBase::rhi_bind()
    {
        if (m_rhi_object != nullptr)
        {
            rhi_object<RHI_RenderTarget>()->bind();
            m_current_target = this;
        }

        return *this;
    }

    const RenderTargetBase& RenderTargetBase::viewport(const ViewPort& viewport)
    {
        if (&m_viewport != &viewport)
            m_viewport = viewport;

        if (m_rhi_object)
        {
            if (is_in_render_thread())
            {
                rhi_object<RHI_RenderTarget>()->viewport(viewport);
            }
            else
            {
                RHI_RenderTarget* rt = rhi_object<RHI_RenderTarget>();
                call_in_render_thread([rt, viewport]() { rt->viewport(viewport); });
            }
        }
        return *this;
    }

    const RenderTargetBase& RenderTargetBase::rhi_clear_color(const ColorClearValue& color, byte layout) const
    {
        if (m_rhi_object)
        {
            rhi_object<RHI_RenderTarget>()->clear_color(color, layout);
        }
        return *this;
    }

    const RenderTargetBase& RenderTargetBase::scissor(const Scissor& scissor)
    {
        if (m_rhi_object)
        {
            if (is_in_render_thread())
            {
                rhi_object<RHI_RenderTarget>()->scissor(scissor);
            }
            else
            {
                RHI_RenderTarget* rt = rhi_object<RHI_RenderTarget>();
                call_in_render_thread([rt, scissor]() { rt->scissor(scissor); });
            }
        }

        if (&scissor != &m_scissor)
            m_scissor = scissor;
        return *this;
    }

    const RenderTargetBase& RenderTargetBase::rhi_clear_depth_stencil(const DepthStencilClearValue& value) const
    {
        if (m_rhi_object)
        {
            rhi_object<RHI_RenderTarget>()->clear_depth_stencil(value);
        }
        return *this;
    }

    RenderTargetBase& RenderTargetBase::rhi_create()
    {
        return *this;
    }

    RenderTargetBase* RenderTargetBase::current_target()
    {
        return m_current_target;
    }

    void RenderTargetBase::reset_current_target()
    {
        m_current_target = nullptr;
    }

    RenderTargetBase::~RenderTargetBase()
    {}

    const ViewPort& RenderTargetBase::viewport() const
    {
        return m_viewport;
    }

    const Scissor& RenderTargetBase::scissor() const
    {
        return m_scissor;
    }

    byte RenderTargetBase::frame_index() const
    {
        trinex_always_check(is_in_render_thread(), "frame_index should only be called in the rendering thread!");
        return m_frame_index;
    }
}// namespace Engine
