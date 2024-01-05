#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/render_target.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(RenderTarget, "Engine", 0);
    implement_default_initialize_class(RenderTarget);


    RenderTarget::RenderTarget() : size({0, 0})
    {}

    RenderTarget& RenderTarget::rhi_create()
    {
        Super::rhi_create();
        _M_rhi_object.reset(EngineInstance::instance()->rhi()->create_render_target(this));
        return *this;
    }


    RenderTarget::Frame::~Frame() = default;

    RenderTarget& RenderTarget::push_frame(Frame* frame)
    {
        _M_frames.push_back(frame);
        return *this;
    }

    RenderTarget& RenderTarget::clear_frames(bool delete_frames)
    {
        if (delete_frames)
        {
            for (Frame* frame : _M_frames)
            {
                delete frame;
            }
        }

        _M_frames.clear();
        return *this;
    }

    RenderTarget::Frame* RenderTarget::current_frame() const
    {
        return frame(_M_frame_index);
    }

    RenderTarget::Frame* RenderTarget::frame(byte index) const
    {
        if (static_cast<byte>(_M_frames.size()) > index)
            return _M_frames[index];
        return nullptr;
    }

    size_t RenderTarget::frames_count() const
    {
        return _M_frames.size();
    }


    RenderTarget::~RenderTarget()
    {
        clear_frames(true);
    }
}// namespace Engine
