#pragma once
#include <engine.hpp>
#include <BasicFunctional/reference_wrapper.hpp>
#include <BasicFunctional/engine_types.hpp>


namespace Engine
{
    class BasicFrameBuffer
    {
    protected:
        ReferenceWrapper<ObjectID> _M_framebuffer_id = ReferenceWrapper<ObjectID>(0);

    public:
        const BasicFrameBuffer& clear_buffer(const BufferType& type = 3) const;
        const BasicFrameBuffer& bind() const;
        const BasicFrameBuffer& view_port(const Point2D& pos,
                                          const Point2D& size) const;
    };
}// namespace Engine
