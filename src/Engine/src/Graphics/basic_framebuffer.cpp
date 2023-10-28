#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Graphics/basic_render_target.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{

    implement_class(BasicRenderTarget, "Engine");
    implement_default_initialize_class(BasicRenderTarget);


    BasicRenderTarget::BasicRenderTarget()
    {}


    const BasicRenderTarget& BasicRenderTarget::bind(size_t buffer_index) const
    {
        if (_M_rhi_render_target)
            _M_rhi_render_target->bind();
        return *this;
    }

    const BasicRenderTarget& BasicRenderTarget::viewport(const ViewPort& viewport)
    {
        if (&_M_viewport != &viewport)
            _M_viewport = viewport;
        if (_M_rhi_render_target)
            _M_rhi_render_target->viewport(viewport);
        return *this;
    }

    const BasicRenderTarget& BasicRenderTarget::clear_color(const ColorClearValue& color, byte layout) const
    {
        if (_M_rhi_render_target)
            _M_rhi_render_target->clear_color(color, layout);
        return *this;
    }

    const BasicRenderTarget& BasicRenderTarget::scissor(const Scissor& scissor)
    {
        if (_M_rhi_render_target)
            _M_rhi_render_target->scissor(scissor);

        if (&scissor != &_M_scissor)
            _M_scissor = scissor;
        return *this;
    }

    const BasicRenderTarget& BasicRenderTarget::clear_depth_stencil(const DepthStencilClearValue& value) const
    {
        if (_M_rhi_render_target)
            _M_rhi_render_target->clear_depth_stencil(value);
        return *this;
    }

    const ViewPort& BasicRenderTarget::viewport()
    {
        return _M_viewport;
    }

    const Scissor& BasicRenderTarget::scissor()
    {
        return _M_scissor;
    }
}// namespace Engine
