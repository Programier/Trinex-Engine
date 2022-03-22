#include "basic_texturearray.hpp"
#include <GL/glew.h>

namespace Engine
{
    namespace basic_texturearray
    {
        static unsigned int prepare(const std::size_t& s, const glm::vec2& max_size)
        {
            unsigned int _M_ID;
            glGenTextures(1, &_M_ID);
            glBindTexture(GL_TEXTURE_2D_ARRAY, _M_ID);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            int w = static_cast<int>(max_size.x), h = static_cast<int>(max_size.y);
            glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, w, h, s);
            return _M_ID;
        }

        unsigned int gen_texture_array(const std::vector<Image*>& images, const glm::vec2& max_size)
        {
            auto _M_ID = prepare(images.size(), max_size);

            int index = 0;
            for (auto& image : images)
            {
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index++, image->width(),
                                image->height(), 1, (image->channels() == 3 ? GL_RGB : GL_RGBA),
                                GL_UNSIGNED_BYTE, image->data());
            }

            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            return _M_ID;
        }

        unsigned int gen_texture_array(const std::list<Image*>& images, const glm::vec2& max_size)
        {
            auto _M_ID = prepare(images.size(), max_size);

            int index = 0;
            for (auto& image : images)
            {
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index++, image->width(),
                                image->height(), 1, (image->channels() == 3 ? GL_RGB : GL_RGBA),
                                GL_UNSIGNED_BYTE, image->data());
            }

            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            return _M_ID;
        }

        unsigned int gen_texture_array(const std::vector<Image>& images, const glm::vec2& max_size)
        {
            auto _M_ID = prepare(images.size(), max_size);

            int index = 0;
            for (auto& image : images)
            {
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index++, image.width(),
                                image.height(), 1, (image.channels() == 3 ? GL_RGB : GL_RGBA),
                                GL_UNSIGNED_BYTE, image.data());
            }

            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            return _M_ID;
        }

        unsigned int gen_texture_array(const std::list<Image>& images, const glm::vec2& max_size)
        {
            auto _M_ID = prepare(images.size(), max_size);

            int index = 0;
            for (auto& image : images)
            {
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index++, image.width(),
                                image.height(), 1, (image.channels() == 3 ? GL_RGB : GL_RGBA),
                                GL_UNSIGNED_BYTE, image.data());
            }

            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            return _M_ID;
        }
    }// namespace basic_texturearray
}// namespace Engine
