#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/render_thread.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_target_base.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(RenderTargetBase, "Engine", 0);
    implement_default_initialize_class(RenderTargetBase);

    RenderTargetBase* RenderTargetBase::_M_current = nullptr;

    RenderTargetBase::RenderTargetBase()
    {}

    RenderTargetBase& RenderTargetBase::rhi_bind()
    {
        if (_M_rhi_object)
        {
            _M_frame_index  = static_cast<byte>(rhi_object<RHI_RenderTarget>()->bind());
            _M_current      = this;
            global_ubo.size = glm::min(_M_viewport.size, _M_scissor.size);
            _M_uniform_buffer->rhi_update(0, sizeof(global_ubo), reinterpret_cast<const byte*>(&global_ubo));
        }
        return *this;
    }

    const RenderTargetBase& RenderTargetBase::viewport(const ViewPort& viewport)
    {
        if (&_M_viewport != &viewport)
            _M_viewport = viewport;

        if (_M_rhi_object)
        {
            RHI_RenderTarget* rt = rhi_object<RHI_RenderTarget>();
            call_in_render_thread([rt, viewport]() { rt->viewport(viewport); });
        }
        return *this;
    }

    const RenderTargetBase& RenderTargetBase::clear_color(const ColorClearValue& color, byte layout) const
    {
        if (_M_rhi_object)
        {
            RHI_RenderTarget* rt = rhi_object<RHI_RenderTarget>();
            call_in_render_thread([rt, color, layout]() { rt->clear_color(color, layout); });
        }
        return *this;
    }

    const RenderTargetBase& RenderTargetBase::scissor(const Scissor& scissor)
    {
        if (_M_rhi_object)
        {
            RHI_RenderTarget* rt = rhi_object<RHI_RenderTarget>();
            call_in_render_thread([rt, scissor]() { rt->scissor(scissor); });
        }

        if (&scissor != &_M_scissor)
            _M_scissor = scissor;
        return *this;
    }

    const RenderTargetBase& RenderTargetBase::clear_depth_stencil(const DepthStencilClearValue& value) const
    {
        if (_M_rhi_object)
        {
            RHI_RenderTarget* rt = rhi_object<RHI_RenderTarget>();
            call_in_render_thread([rt, value]() { rt->clear_depth_stencil(value); });
        }
        return *this;
    }

    RenderTargetBase& RenderTargetBase::rhi_create()
    {
        if (_M_uniform_buffer == nullptr)
        {
            UniformBuffer* ubo = Object::new_instance<UniformBuffer>();
            ubo->init_data     = reinterpret_cast<const byte*>(&global_ubo);
            ubo->init_size     = sizeof(global_ubo);

            ubo->init_resource();
            _M_uniform_buffer = ubo;
        }
        return *this;
    }

    UniformBuffer* RenderTargetBase::uniform_buffer() const
    {
        return _M_uniform_buffer.ptr();
    }

    RenderTargetBase* RenderTargetBase::current_target()
    {
        return _M_current;
    }

    void RenderTargetBase::reset_current_target()
    {
        _M_current = nullptr;
    }

    RenderTargetBase::~RenderTargetBase()
    {}

    const ViewPort& RenderTargetBase::viewport()
    {
        return _M_viewport;
    }

    const Scissor& RenderTargetBase::scissor()
    {
        return _M_scissor;
    }
}// namespace Engine
