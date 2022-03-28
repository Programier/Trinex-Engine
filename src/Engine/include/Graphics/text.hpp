#pragma once
#include <glm/glm.hpp>
#include <map>
#include <string>


namespace Engine
{
    class Text
    {
        struct Character {
            unsigned int texture_id;// ID текстуры глифа
            glm::ivec2 size;        // Размеры глифа
            glm::ivec2 bearing;     // Смещение верхней левой точки глифа
            unsigned int advance;// Горизонтальное смещение до начала следующего глифа
        };

        std::map<char, Character> _M_characters;
        void* _M_ft = nullptr;
        void* _M_face = nullptr;
        unsigned int _M_VAO, _M_VBO;

    public:
        Text(std::string font = "/usr/share/fonts/google-noto/NotoSans-CondensedBlackItalic.ttf", float font_size = 48);
        void draw(std::string text, float x, float y, float scale);
        ~Text();
    };
}// namespace Engine
