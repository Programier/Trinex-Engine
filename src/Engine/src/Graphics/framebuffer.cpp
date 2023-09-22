#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/framebuffer.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(FrameBuffer, "Engine");
    implement_default_initialize_class(FrameBuffer);

    FrameBuffer::FrameBuffer()
    {}

    FrameBuffer& FrameBuffer::create(const FrameBufferCreateInfo& info)
    {
        ApiObject::destroy();
        _M_rhi_framebuffer = EngineInstance::instance()->api_interface()->create_framebuffer(info);
        return *this;
    }
}// namespace Engine
