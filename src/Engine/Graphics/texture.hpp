#pragma once
#include "../Image/image.hpp"
#include <string>


namespace Engine
{
    enum DrawMode
    {
        NEAREST,
        LINEAR
    };

    class Texture : Image
    {
        unsigned int _M_ID = 0;

        DrawMode _M_mode;
        void private_load(const std::string& name, const DrawMode& mode, const bool& invert);

    public:
        Texture();
        Texture(const Texture& texture);
        Texture(const std::string& texture, const DrawMode& mode = NEAREST,
                const bool& invert = true);

        Texture& operator=(const Texture&);
        Texture& load(const std::string& texture, const DrawMode& mode = NEAREST,
                      const bool& invert = true);

        Texture& draw_mode(const DrawMode& mode);
        const DrawMode& draw_mode();
        Texture& bind();
        static void unbind();

        ~Texture();
    };

}// namespace Engine
