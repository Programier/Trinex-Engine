#include <GL/glew.h>
#include <Graphics/basic_texturearray.hpp>
#include <unordered_map>

namespace Engine
{
    namespace basic_texturearray
    {
        static ObjectID prepare(const std::size_t& s, const Size2D& size)
        {
            unsigned int _M_ID;
            glGenTextures(1, &_M_ID);
            glBindTexture(GL_TEXTURE_2D_ARRAY, _M_ID);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            int w = static_cast<int>(size.x), h = static_cast<int>(size.y);
            glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, w, h, s);
            return _M_ID;
        }

        template<typename Container>
        ObjectID get_texture_from_pointer_container(const Container& images, const Size2D& max_size)
        {
            auto _M_ID = prepare(images.size(), max_size);
            std::unordered_map<Image*, bool> _M_loaded;
            int index = 0;
            for (auto& image : images)
            {
                if (_M_loaded[image] == false)
                {
                    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index++, image->width(), image->height(), 1,
                                    (image->channels() == 3 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, image->data());
                    _M_loaded[image] = true;
                }
            }

            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            return _M_ID;
        }

        template<typename Container>
        ObjectID get_texture_from_objects_container(const Container& images, const Size2D& max_size)
        {
            auto _M_ID = prepare(images.size(), max_size);

            int index = 0;
            for (auto& image : images)
            {
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index++, image.width(), image.height(), 1,
                                (image.channels() == 3 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, image.data());
            }

            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            return _M_ID;
        }


        ObjectID gen_texture_array(const std::vector<Image*>& images, const Size2D& max_size)
        {
            return get_texture_from_pointer_container(images, max_size);
        }

        ObjectID gen_texture_array(const std::list<Image*>& images, const Size2D& max_size)
        {
            return get_texture_from_pointer_container(images, max_size);
        }

        ObjectID gen_texture_array(const DynamicArray<Image*>& images, const Size2D& max_size)
        {
            return get_texture_from_pointer_container(images, max_size);
        }

        ObjectID gen_texture_array(const std::vector<Image>& images, const Size2D& max_size)
        {
            return get_texture_from_objects_container(images, max_size);
        }

        ObjectID gen_texture_array(const std::list<Image>& images, const Size2D& max_size)
        {
            return get_texture_from_objects_container(images, max_size);
        }

        ObjectID gen_texture_array(const DynamicArray<Image>& images, const Size2D& max_size)
        {
            return get_texture_from_objects_container(images, max_size);
        }
    }// namespace basic_texturearray
}// namespace Engine
