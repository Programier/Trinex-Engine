#include "texture.hpp"
#include <GL/glew.h>
#include <iostream>
#include <stdexcept>

#define str(a) std::string(a)
namespace Engine
{

    void Texture::private_load(const std::string& name, const DrawMode& mode, const bool& invert)
    {
        _M_mode = mode;
        Image::load(name, invert);
        if (Image::empty())
            throw std::runtime_error("Texture: Failed to load texture " + name);

        unsigned int ID;
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Image::width(), Image::height(), 0,
                     (Image::channels() == 3 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE,
                     (GLvoid*) Image::data());
        auto gl_mode = mode == NEAREST ? GL_NEAREST : GL_LINEAR;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_mode);
        glBindTexture(GL_TEXTURE_2D, 0);
        _M_ID = ID;
    }

    Texture::Texture(const std::string& file, const DrawMode& mode, const bool& invert)
    {
        private_load(file, mode, invert);
    }

    void Texture::unbind()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }


    Texture& Texture::bind()
    {
        glBindTexture(GL_TEXTURE_2D, _M_ID);
        return *this;
    }


    Texture::~Texture()
    {
        if (_M_ID != 0)
            glDeleteTextures(1, &_M_ID);
    }

    Texture& Texture::draw_mode(const DrawMode& mode)
    {
        _M_mode = mode;

        glBindTexture(GL_TEXTURE_2D, _M_ID);
        auto gl_mode = mode == NEAREST ? GL_NEAREST : GL_LINEAR;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_mode);
        glBindTexture(GL_TEXTURE_2D, 0);

        return *this;
    }

    const DrawMode& Texture::draw_mode()
    {
        return _M_mode;
    }

    Texture& Texture::load(const std::string& texture, const DrawMode& mode, const bool& invert)
    {
        if (_M_ID != 0)
            glDeleteTextures(1, &_M_ID);
        private_load(texture, mode, invert);
        return *this;
    }


    Texture::Texture() = default;
    Texture& Texture::operator=(const Texture& texture)
    {
        if (this == &texture)
            return *this;
        if (_M_ID != 0)
            glDeleteTextures(1, &_M_ID);
        dynamic_cast<Image&>(*this) = dynamic_cast<const Image&>(texture);
        _M_mode = texture._M_mode;

        unsigned int ID;
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Image::width(), Image::height(), 0,
                     (Image::channels() == 3 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE,
                     (GLvoid*) Image::data());
        auto gl_mode = _M_mode == NEAREST ? GL_NEAREST : GL_LINEAR;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_mode);
        glBindTexture(GL_TEXTURE_2D, 0);
        _M_ID = ID;

        return *this;
    }

    Texture::Texture(const Texture& texture)
    {
        *this = texture;
    }


}// namespace Engine
