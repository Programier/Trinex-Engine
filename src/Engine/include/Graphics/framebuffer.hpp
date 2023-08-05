#pragma once
#include <Core/buffer_types.hpp>
#include <Core/implement.hpp>
#include <Graphics/basic_framebuffer.hpp>
#include <Graphics/texture_2D.hpp>


namespace Engine
{
    class ENGINE_EXPORT FrameBuffer : public BasicFrameBuffer
    {
    public:
        using Super = BasicFrameBuffer;

        delete_copy_constructors(FrameBuffer);
        FrameBuffer();
        FrameBuffer& create(const FrameBufferCreateInfo&);
        static ENGINE_EXPORT FrameBuffer* g_buffer();

        static void on_class_register(void*);
    };
}// namespace Engine
