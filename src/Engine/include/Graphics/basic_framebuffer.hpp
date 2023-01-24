#pragma once
#include <Core/api_object.hpp>
#include <Core/engine_types.hpp>
#include <Core/export.hpp>


namespace Engine
{
    CLASS BasicFrameBuffer : public ApiObject
    {
    protected:
        FrameBufferType _M_type = FrameBufferType::FRAMEBUFFER;

        declare_instance_info_hpp(BasicFrameBuffer);

    public:
        delete_copy_constructors(BasicFrameBuffer);
        constructor_hpp(BasicFrameBuffer);
        const BasicFrameBuffer& clear_buffer(const BufferType& type = 3) const;
        const BasicFrameBuffer& bind() const;
        const BasicFrameBuffer& view_port(const Point2D& pos, const Size2D& size) const;
        FrameBufferType type() const;
    };
}// namespace Engine
