#pragma once
#include <engine.hpp>
#include <BasicFunctional/reference_wrapper.hpp>
#include <Graphics/basic_framebuffer.hpp>
#include <Window/window.hpp>

namespace Engine
{
    class FrameBuffer : public BasicFrameBuffer
    {
        ReferenceWrapper<ObjectID> _M_texture_id;
        Size2D _M_size;
        FrameBuffer& delete_data();

    public:
        FrameBuffer();
        FrameBuffer(const Size1D& width, const Size1D& height);
        FrameBuffer(const Size2D& size);
        FrameBuffer(const FrameBuffer& buffer);
        FrameBuffer& operator=(const FrameBuffer& buffer);
        FrameBuffer& gen(const Size1D& width, const Size1D& height);
        FrameBuffer& gen(const Size2D& size);
        FrameBuffer& bind_texture(int num = 1);
        Size1D width() const;
        Size1D height() const;
        Size2D size() const;
        ~FrameBuffer();
    };
}// namespace Engine
