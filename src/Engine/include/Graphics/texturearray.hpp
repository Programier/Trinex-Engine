#pragma once

#include <Graphics/texture.hpp>
#include <Image/image.hpp>
#include <BasicFunctional/engine_types.hpp>
#include <list>
#include <string>


namespace Engine
{

    class TextureArray
    {
        ObjectID _M_ID = 0;
        std::vector<Image> _M_images;
        DrawMode _M_mode;
        Size2D _M_max_size;
        MipMapLevel _M_mipmap;
        void create();

    public:
        TextureArray();
        TextureArray(const TextureArray&);

        TextureArray(const std::vector<std::string>& textures, const DrawMode& mode = Engine::DrawMode::NEAREST,
                     const MipMapLevel& mipmap = 4, const bool& invert = true);
        TextureArray(const std::list<std::string>& textures, const DrawMode& mode = Engine::DrawMode::NEAREST,
                     const MipMapLevel& mipmap = 4, const bool& invert = true);
        TextureArray& operator=(const TextureArray&);

        TextureArray& load(const std::vector<std::string>& textures, const DrawMode& mode = Engine::DrawMode::NEAREST,
                           const MipMapLevel& mipmap = 4, const bool& invert = true);
        TextureArray& load(const std::list<std::string>& textures, const DrawMode& mode = Engine::DrawMode::NEAREST,
                           const MipMapLevel& mipmap = 4, const bool& invert = true);
        TextureArray& draw_mode(const DrawMode& mode);
        const DrawMode& draw_mode();
        TextureArray& bind();
        static void unbind();
        Size2D size();
        const std::vector<Image>& images() const;
        ~TextureArray();
    };
}// namespace Engine
