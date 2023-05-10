#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/framebuffer.hpp>
#include <api.hpp>

namespace Engine
{

    REGISTER_CLASS(Engine::FrameBuffer, Engine::BasicFrameBuffer);

    FrameBuffer::FrameBuffer()
    {}


    FrameBuffer& FrameBuffer::create(const FrameBufferCreateInfo& info)
    {
        ApiObject::destroy();
        EngineInstance::instance()->api_interface()->gen_framebuffer(_M_ID, info);
        return *this;
    }


    const Vector<Texture2D*> FrameBuffer::textures() const
    {
        return _M_textures;
    }
}// namespace Engine
