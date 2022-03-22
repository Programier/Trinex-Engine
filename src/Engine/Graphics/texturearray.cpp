#include "texturearray.hpp"
#include "basic_texturearray.hpp"
#include <GL/glew.h>
#include <iostream>
#include <stdexcept>

#define img _M_images.back()
#define THROW throw std::runtime_error("TextureArray: Failed to create texture array")

namespace Engine
{

    void TextureArray::create()
    {
        _M_ID = Engine::basic_texturearray::gen_texture_array(
                _M_images, glm::vec2((float) _M_max_width, (float) _M_max_height));
        bind();
        auto m = _M_mode == LINEAR ? GL_LINEAR : GL_NEAREST;
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, m);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, m);
        unbind();
    }


    TextureArray::TextureArray() = default;

    TextureArray::TextureArray(const TextureArray& t)
    {
        *this = t;
    }

    TextureArray::TextureArray(const std::vector<std::string>& textures, const DrawMode& mode,
                               const bool& invert)
    {
        load(textures, mode, invert);
    }

    TextureArray& TextureArray::operator=(const TextureArray& texture_array)
    {
        if (this == &texture_array)
            return *this;
        _M_mode = texture_array._M_mode;
        _M_images = texture_array._M_images;

        if (_M_ID != 0)
            glDeleteTextures(1, &_M_ID);
        create();
        return *this;
    }

    TextureArray& TextureArray::load(const std::vector<std::string>& textures, const DrawMode& mode,
                                     const bool& invert)
    {
        if (textures.size() == 0)
            return *this;
        _M_mode = mode;
        _M_max_width = _M_max_height = 0;
        if (_M_ID != 0)
            glDeleteTextures(1, &_M_ID);
        _M_images.clear();

        // Loading images
        std::clog << "TextureArray: Loading " << textures[0] << std::endl;
        _M_images.reserve(textures.size());
        _M_images.push_back(Image());
        img.load(textures[0], invert);
        _M_max_width = img.width();
        _M_max_height = img.height();
        if (img.empty())
            THROW;

        auto len = textures.size();
        for (decltype(len) i = 1; i < len; i++)
        {
            auto& texture = textures[i];
            std::clog << "TextureArray: Loading " << texture << std::endl;
            _M_images.push_back(Image());
            img.load(texture, invert);
            if (img.empty() || img.width() != _M_max_width || img.height() != _M_max_height)
                THROW;
        }

        create();
        return *this;
    }

    TextureArray& TextureArray::draw_mode(const DrawMode& mode)
    {
        _M_mode = mode;
        bind();
        auto m = _M_mode == LINEAR ? GL_LINEAR : GL_NEAREST;
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, m);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, m);
        unbind();
        return *this;
    }

    const DrawMode& TextureArray::draw_mode()
    {
        return _M_mode;
    }

    TextureArray& TextureArray::bind()
    {
        if (_M_ID != 0)
        {
            glBindTexture(GL_TEXTURE_2D_ARRAY, _M_ID);
        }
        return *this;
    }

    TextureArray::~TextureArray()
    {
        if (_M_ID != 0)
            glDeleteTextures(1, &_M_ID);
    }

    void TextureArray::unbind()
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    glm::vec2 TextureArray::get_max_size()
    {
        return glm::vec2(_M_max_width, _M_max_height);
    }

    const std::vector<Image>& TextureArray::images() const
    {
        return _M_images;
    }
}// namespace Engine
