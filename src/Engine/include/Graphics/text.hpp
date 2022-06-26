#pragma once
#include <BasicFunctional/engine_types.hpp>
#include <map>
#include <string>


namespace Engine
{
    class Text
    {
        struct Character {
            ObjectID texture_id;// ID текстуры глифа
            glm::ivec2 size;    // Размеры глифа
            glm::ivec2 bearing; // Смещение верхней левой точки глифа
            unsigned int advance;// Горизонтальное смещение до начала следующего глифа
        };

        std::map<unsigned long, Character> _M_characters;
        void* _M_ft = nullptr;
        void* _M_face = nullptr;
        ObjectID _M_VAO = 0, _M_VBO = 0;
        Size2D _M_size;
        std::string _M_font_path;

        Text& init();
        Text& terminate();

        Text& push_char(unsigned long ch);

    public:
        Text(const std::string& font = "/usr/share/fonts/google-noto/NotoSans-CondensedBlackItalic.ttf",
             const Size2D& size = {0.f, 48.f});
        Text& draw(const std::string& text, Size1D x, Size1D y, float scale = 1.f);
        Text& draw(const std::string& text, const Size2D& pos, float scale = 1.f);
        Text& draw(const std::wstring& text, Size1D x, Size1D y, float scale = 1.f);
        Text& draw(const std::wstring& text, const Size2D& pos, float scale = 1.f);
        const Size2D& font_size() const;
        Text& font_size(const Size2D& size);
        const std::string& font_path() const;
        Text& font_path(const std::string& path);
        ~Text();
    };
}// namespace Engine
