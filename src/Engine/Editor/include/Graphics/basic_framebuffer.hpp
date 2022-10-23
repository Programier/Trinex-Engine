#pragma once
#include <Core/object.hpp>
#include <Core/engine_types.hpp>
#include <Core/export.hpp>


namespace Engine
{
    CLASS BasicFrameBuffer : public Object
    {
    protected:
        FrameBufferType _M_type = FrameBufferType::FRAMEBUFFER;

    public:
        const BasicFrameBuffer& clear_buffer(const BufferType& type = 3) const;
        const BasicFrameBuffer& bind() const;
        const BasicFrameBuffer& view_port(const Point2D& pos,
                                          const Size2D& size) const;
        FrameBufferType type() const;
    };
}// namespace Engine
