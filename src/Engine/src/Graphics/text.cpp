#include <Graphics/text.hpp>
#include <ft2build.h>
#include <iostream>
#include <opengl.hpp>
#include FT_FREETYPE_H

namespace Engine
{
    Text& Text::init()
    {
        FT_Library ft = nullptr;
        FT_Face face = nullptr;
        if (FT_Init_FreeType(&ft))
            throw std::runtime_error("ERROR::FREETYPE: Could not init FreeType Library");

        if (FT_New_Face(ft, _M_font_path.c_str(), 0, &face))
            throw std::runtime_error("ERROR::FREETYPE: Failed to load font");

        _M_ft = static_cast<void*>(ft);
        _M_face = static_cast<void*>(face);

        // Initialization
        FT_Set_Pixel_Sizes(face, static_cast<FT_UInt>(_M_size.x), static_cast<FT_UInt>(_M_size.y));
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);// Disable byte-alignment restriction

        for (GLubyte c = 0; c < 128; c++) push_char(c);

        glGenVertexArrays(1, &_M_VAO);
        glGenBuffers(1, &_M_VBO);
        glBindVertexArray(_M_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, _M_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        return *this;
    }

    Text& Text::terminate()
    {
        FT_Library ft = static_cast<FT_Library>(_M_ft);
        FT_Face face = static_cast<FT_Face>(_M_face);

        if (face)
        {
            FT_Done_Face(face);
            _M_ft = nullptr;
        }

        if (ft)
        {
            FT_Done_FreeType(ft);
            _M_face = nullptr;
        }

        glDeleteVertexArrays(1, &_M_VAO);
        glDeleteBuffers(1, &_M_VBO);
        _M_VAO = _M_VBO = 0;
        for (auto& ell : _M_characters) glDeleteTextures(1, &ell.second.texture_id);

        return *this;
    }

    Text& Text::push_char(unsigned long c)
    {
        FT_Face face = static_cast<FT_Face>(_M_face);
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FREETYTPE: Failed to load symbol with ASCII value  " << (int) c << std::endl;
            return *this;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED,
                     GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {texture, glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                               glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                               (unsigned int) face->glyph->advance.x};
        _M_characters.insert(std::pair<unsigned long, Character>(c, character));

        return *this;
    }

    Text::Text(const std::string& font, const Size2D& font_size) : _M_size(font_size), _M_font_path(font)
    {
        init();
    }

    Text::~Text()
    {
        terminate();
    }

    Text& Text::draw(const std::string& text, Size1D x, Size1D y, float scale)
    {
        // Activate corresponding render state
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(_M_VAO);

        for (char c : text)
        {
            Character* ch_ptr = nullptr;
            try
            {
                ch_ptr = &_M_characters.at(c);
            }
            catch (...)
            {
                continue;
            }

            auto& ch = *ch_ptr;

            GLfloat xpos = x + ch.bearing.x * scale;
            GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

            GLfloat w = ch.size.x * scale;
            GLfloat h = ch.size.y * scale;
            // Update VBO for each character
            GLfloat vertices[6][4] = {{xpos, ypos + h, 0.0, 0.0}, {xpos, ypos, 0.0, 1.0},
                                      {xpos + w, ypos, 1.0, 1.0}, {xpos, ypos + h, 0.0, 0.0},
                                      {xpos + w, ypos, 1.0, 1.0}, {xpos + w, ypos + h, 1.0, 0.0}};

            // Render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.texture_id);
            // Update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, _M_VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.advance >> 6) * scale;// Bitshift by 6 to get value in pixels (2^6 = 64)
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        return *this;
    }

    Text& Text::draw(const std::wstring& text, Size1D x, Size1D y, float scale)
    {
        // Activate corresponding render state
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(_M_VAO);

        // Iterate through all characters
        for (const auto& symbol : text)
        {
            unsigned long c = static_cast<unsigned long>(symbol);
            const Character* ch_ptr = nullptr;
            try
            {
                ch_ptr = &_M_characters.at(c);
            }
            catch (const std::exception& e)
            {
                push_char(c);
                try
                {
                    ch_ptr = &_M_characters.at((unsigned long) c);
                }
                catch (const std::exception& e2)
                {
                    std::clog << e2.what() << std::endl;
                    continue;
                }
            }


            auto& ch = *ch_ptr;

            GLfloat xpos = x + ch.bearing.x * scale;
            GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

            GLfloat w = ch.size.x * scale;
            GLfloat h = ch.size.y * scale;
            // Update VBO for each character
            GLfloat vertices[6][4] = {{xpos, ypos + h, 0.0, 0.0}, {xpos, ypos, 0.0, 1.0},
                                      {xpos + w, ypos, 1.0, 1.0}, {xpos, ypos + h, 0.0, 0.0},
                                      {xpos + w, ypos, 1.0, 1.0}, {xpos + w, ypos + h, 1.0, 0.0}};

            // Render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.texture_id);
            // Update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, _M_VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += (ch.advance >> 6) * scale;// Bitshift by 6 to get value in pixels (2^6 = 64)
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        return *this;
    }


    Text& Text::draw(const std::string& text, const Size2D& pos, float scale)
    {
        return draw(text, pos.x, pos.y, scale);
    }

    Text& Text::draw(const std::wstring& text, const Size2D& pos, float scale)
    {
        return draw(text, pos.x, pos.y, scale);
    }

    const Size2D& Text::font_size() const
    {
        return _M_size;
    }

    Text& Text::font_size(const Size2D& size)
    {
        terminate();
        _M_size = size;
        init();
        return *this;
    }

    const std::string& Text::font_path() const
    {
        return _M_font_path;
    }

    Text& Text::font_path(const std::string& path)
    {
        terminate();
        _M_font_path = path;
        init();
        return *this;
    }

}// namespace Engine
