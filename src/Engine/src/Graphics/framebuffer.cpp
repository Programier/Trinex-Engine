#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/framebuffer.hpp>
#include <api.hpp>

namespace Engine
{
    implement_class(FrameBuffer, "Engine");
    implement_default_initialize_class(FrameBuffer);

    FrameBuffer::FrameBuffer()
    {}

    FrameBuffer& FrameBuffer::create(const FrameBufferCreateInfo& info)
    {
        ApiObject::destroy();
        EngineInstance::instance()->api_interface()->gen_framebuffer(_M_ID, info);
        return *this;
    }
}// namespace Engine
