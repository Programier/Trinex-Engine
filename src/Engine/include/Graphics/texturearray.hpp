#pragma once

#include <Graphics/texture.hpp>
#include <Image/image.hpp>
#include <glm/glm.hpp>
#include <list>
#include <string>


namespace Engine
{

    class TextureArray
    {
        unsigned int _M_ID = 0;
        std::vector<Image> _M_images;
        DrawMode _M_mode;
        int _M_max_width = 0, _M_max_height = 0;
        unsigned int _M_mipmap;
        void create();

    public:
        TextureArray();
        TextureArray(const TextureArray&);

        TextureArray(const std::vector<std::string>& textures, const DrawMode& mode = Engine::DrawMode::NEAREST,
                     const unsigned int& mipmap = 4, const bool& invert = true);
        TextureArray(const std::list<std::string>& textures, const DrawMode& mode = Engine::DrawMode::NEAREST,
                     const unsigned int& mipmap = 4, const bool& invert = true);
        TextureArray& operator=(const TextureArray&);

        TextureArray& load(const std::vector<std::string>& textures, const DrawMode& mode = Engine::DrawMode::NEAREST,
                           const unsigned int& mipmap = 4, const bool& invert = true);
        TextureArray& load(const std::list<std::string>& textures, const DrawMode& mode = Engine::DrawMode::NEAREST,
                           const unsigned int& mipmap = 4, const bool& invert = true);
        TextureArray& draw_mode(const DrawMode& mode);
        const DrawMode& draw_mode();
        TextureArray& bind();
        static void unbind();
        glm::vec2 get_size();
        const std::vector<Image>& images() const;
        ~TextureArray();
    };
}// namespace Engine
