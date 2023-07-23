#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/framebuffer.hpp>
#include <api.hpp>

namespace Engine
{

    static void on_init()
    {
        register_class(Engine::FrameBuffer);
    }

    static InitializeController initializer(on_init);


    FrameBuffer::FrameBuffer()
    {}


    FrameBuffer& FrameBuffer::create(const FrameBufferCreateInfo& info)
    {
        ApiObject::destroy();
        EngineInstance::instance()->api_interface()->gen_framebuffer(_M_ID, info);
        return *this;
    }
}// namespace Engine
