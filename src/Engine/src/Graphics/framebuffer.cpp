#include <Graphics/framebuffer.hpp>
#include <api_funcs.hpp>
#include <opengl.hpp>

namespace Engine
{
    declare_instance_info_cpp(FrameBuffer);
    constructor_cpp(FrameBuffer)
    {}

    FrameBuffer& FrameBuffer::gen(FrameBufferType type)
    {
        ApiObject::destroy();
        _M_type = type;
        gen_framebuffer(_M_ID, type);
        return *this;
    }

    FrameBuffer& FrameBuffer::attach_texture(Texture2D* texture, FrameBufferAttach attach, unsigned int num, int level)
    {
        if (!texture)
            return *this;

        attach_texture_to_framebuffer(_M_ID, texture->id(), attach, num, level);
        _M_textures.push_back(texture);
        return *this;
    }


    const std::vector<Texture2D*> FrameBuffer::textures() const
    {
        return _M_textures;
    }
}// namespace Engine
