#pragma once
#include <Image/image.hpp>
#include <string>


namespace Engine
{
    enum DrawMode
    {
        NEAREST,
        LINEAR
    };

    class Texture : public Image
    {
        ObjectID _M_ID = 0;
        MipMapLevel _M_mipmap = 4;

        DrawMode _M_mode;
        Texture& private_load(const std::string& name, const DrawMode& mode, const MipMapLevel& mipmap, const bool& invert);
        Texture& gen_ID();
        Texture& delete_ID();

    public:
        Texture();
        Texture(const Texture& texture);
        Texture(const std::string& texture, const DrawMode& mode = NEAREST, const MipMapLevel& mipmap_level = 4,
                const bool& invert = true);

        Texture& operator=(const Texture&);
        Texture& load(const std::string& texture, const DrawMode& mode = NEAREST, const MipMapLevel& mipmap_level = 4,
                      const bool& invert = true);

        Texture& draw_mode(const DrawMode& mode);
        const DrawMode& draw_mode();
        Texture& bind();
        static void unbind();
        Texture& update();

        ~Texture();
    };

}// namespace Engine
