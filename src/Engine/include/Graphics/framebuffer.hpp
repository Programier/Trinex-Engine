#pragma once
#include <Core/buffer_types.hpp>
#include <Core/implement.hpp>
#include <Graphics/basic_framebuffer.hpp>
#include <Graphics/texture_2D.hpp>


namespace Engine
{
    class ENGINE_EXPORT FrameBuffer : public BasicFrameBuffer
    {
    protected:
        Vector<Texture2D*> _M_textures;

    public:
        delete_copy_constructors(FrameBuffer);
        FrameBuffer();
        FrameBuffer& create(const FrameBufferCreateInfo&);
        const Vector<Texture2D*> textures() const;
    };
}// namespace Engine
