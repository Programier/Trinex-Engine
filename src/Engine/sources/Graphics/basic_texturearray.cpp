#include <Graphics/basic_texturearray.hpp>
#include <GL/glew.h>
#include <unordered_map>

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

        template<typename Container>
        unsigned int get_texture_from_pointer_container(const Container& images,
                                                        const glm::vec2& max_size)
        {
            auto _M_ID = prepare(images.size(), max_size);
            std::unordered_map<Image*, bool> _M_loaded;
            int index = 0;
            for (auto& image : images)
            {
                if (_M_loaded[image] == false)
                {
                    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index++, image->width(),
                                    image->height(), 1, (image->channels() == 3 ? GL_RGB : GL_RGBA),
                                    GL_UNSIGNED_BYTE, image->data());
                    _M_loaded[image] = true;
                }
            }

            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            return _M_ID;
        }

        template<typename Container>
        unsigned int get_texture_from_objects_container(const Container& images,
                                                        const glm::vec2& max_size)
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


        unsigned int gen_texture_array(const std::vector<Image*>& images, const glm::vec2& max_size)
        {
            return get_texture_from_pointer_container(images, max_size);
        }

        unsigned int gen_texture_array(const std::list<Image*>& images, const glm::vec2& max_size)
        {
            return get_texture_from_pointer_container(images, max_size);
        }

        unsigned int gen_texture_array(const std::vector<Image>& images, const glm::vec2& max_size)
        {
            return get_texture_from_objects_container(images, max_size);
        }

        unsigned int gen_texture_array(const std::list<Image>& images, const glm::vec2& max_size)
        {
            return get_texture_from_objects_container(images, max_size);
        }
    }// namespace basic_texturearray
}// namespace Engine
