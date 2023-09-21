#pragma once
#include <Core/implement.hpp>
#include <Graphics/basic_framebuffer.hpp>
#include <Graphics/texture_2D.hpp>


namespace Engine
{
    struct FrameBufferCreateInfo;

    class ENGINE_EXPORT FrameBuffer : public BasicFrameBuffer
    {
        declare_class(FrameBuffer, BasicFrameBuffer);
    public:
        delete_copy_constructors(FrameBuffer);
        FrameBuffer();
        FrameBuffer& create(const FrameBufferCreateInfo&);
        static ENGINE_EXPORT FrameBuffer* g_buffer();
    };
}// namespace Engine
