#pragma once
#include <Graphics/basic_framebuffer.hpp>
#include <Core/implement.hpp>
#include <Graphics/texture_2D.hpp>
#include <vector>

namespace Engine
{
    CLASS FrameBuffer : public BasicFrameBuffer
    {
    protected:
        std::vector<Texture2D*> _M_textures;

    declare_instance_info_hpp(FrameBuffer);
    public:
        delete_copy_constructors(FrameBuffer);
        constructor_hpp(FrameBuffer);
        FrameBuffer& gen(FrameBufferType type = FrameBufferType::FRAMEBUFFER);
        FrameBuffer& attach_texture(Texture2D* texture, FrameBufferAttach attach, unsigned int num = 0, int level = 0);
        const std::vector<Texture2D*> textures() const;
    };
}// namespace Engine
