#include <Core/logger.hpp>
#include <Graphics/font.hpp>
#include <ft2build.h>
#include <stdexcept>
#include FT_FREETYPE_H

namespace Engine
{


    static void terminate_FT_Library(void* address)
    {
        FT_Library ft = static_cast<FT_Library>(address);
        if (ft)
            FT_Done_FreeType(ft);
    }

    static void terminate_FT_Face(void* address)
    {
        FT_Face ft = static_cast<FT_Face>(address);
        if (ft)
            FT_Done_Face(ft);
    }

    declare_instance_info_cpp(Font);
    constructor_cpp(Font)
    {}

    constructor_cpp(Font, const std::string& font, const Size2D& size)
    {
        load(font, size);
    }

    Font& Font::load(const std::string& font, const Size2D& size)
    {
        terminate();
        _M_size = size;
        _M_font_path = font;
        FT_Library ft = nullptr;
        FT_Face face = nullptr;
        if (FT_Init_FreeType(&ft))
            throw std::runtime_error("ERROR::FREETYPE: Could not init FreeType Library");

        if (FT_New_Face(ft, _M_font_path.c_str(), 0, &face))
            throw std::runtime_error("ERROR::FREETYPE: Failed to load font");

        _M_ft = SmartPointer<void>(static_cast<void*>(ft), terminate_FT_Library);
        _M_face = SmartPointer<void>(static_cast<void*>(face), terminate_FT_Face);

        // Initialization
        FT_Set_Pixel_Sizes(face, static_cast<FT_UInt>(_M_size.x), static_cast<FT_UInt>(_M_size.y));

        for (byte c = 0; c < 128; c++) push_char(c);

        _M_mesh.data.resize(24);
        _M_mesh.indexes = {0, 1, 2, 3, 4, 5};
        _M_mesh.gen();
        _M_mesh.set_data();
        _M_mesh.set_indexes();
        return *this;
    }

    Font& Font::terminate()
    {
        _M_characters.clear();
        _M_face = nullptr;
        _M_ft = nullptr;
        _M_font_path = "";
        return *this;
    }

    Font& Font::push_char(wchar_t c)
    {
        try
        {
            _M_characters.at(c);
            return *this;
        }
        catch (...)
        {}

        FT_Face face = static_cast<FT_Face>(_M_face.get());
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            logger->log("ERROR::FREETYTPE: Failed to load symbol with ASCII value %d\n", int(c));
            return *this;
        }

        // Generate Fonture
        Character character = {
                Object::new_instance<Texture2D>(), glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top), (unsigned int) face->glyph->advance.x};

        TextureParams params;
        params.pixel_type = PixelType::Red;
        params.pixel_component_type = PixelComponentType::UnsignedByte;
        params.type = TextureType::Texture2D;

        character.gliph->create(params);
        character.gliph->gen({face->glyph->bitmap.width, face->glyph->bitmap.rows}, 0, face->glyph->bitmap.buffer)
                .mag_filter(TextureFilter::Linear)
                .min_filter(TextureFilter::Linear);


        // Now store character for later use
        _M_characters.insert(std::pair<unsigned long, Character>(c, character));
        return *this;
    }


    Font::~Font()
    {
        terminate();
    }


    Font& Font::draw(const std::string& text, Size1D x, Size1D y, float scale)
    {
        Size2D orig = {x, y};

        for (char c : text)
        {
            if (c == '\n')
            {
                y -= this->_M_size.y;
                x = orig.x;
                continue;
            }

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

            float xpos = x + ch.bearing.x * scale;
            float ypos = y - (ch.size.y - ch.bearing.y) * scale;

            float w = ch.size.x * scale;
            float h = ch.size.y * scale;
            // Update VBO for each character
            _M_mesh.data = {xpos, ypos + h, 0.0, 0.0, xpos,     ypos, 0.0, 1.0, xpos + w, ypos,     1.0, 1.0,
                            xpos, ypos + h, 0.0, 0.0, xpos + w, ypos, 1.0, 1.0, xpos + w, ypos + h, 1.0, 0.0};
            _M_mesh.update_data(0, _M_mesh.data.size() * sizeof(float));

            // Render glyph Fonture over quad
            ch.gliph->bind();
            _M_mesh.draw(Primitive::Triangle, 6, 0);
            x += (ch.advance >> 6) * scale;// Bitshift by 6 to get value in pixels (2^6 = 64)
        }

        return *this;
    }

    Font& Font::draw(const std::wstring& Font, Size1D x, Size1D y, float scale)
    {
        Size2D orig = {x, y};
        // Iterate through all characters
        for (const auto& symbol : Font)
        {
            if (symbol == L'\n')
            {
                y -= this->_M_size.y;
                x = orig.x;
                continue;
            }
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
                    logger->log("%s\n", e2.what());
                    continue;
                }
            }


            auto& ch = *ch_ptr;

            float xpos = x + ch.bearing.x * scale;
            float ypos = y - (ch.size.y - ch.bearing.y) * scale;

            float w = ch.size.x * scale;
            float h = ch.size.y * scale;

            // Update VBO for each character
            _M_mesh.data = {xpos, ypos + h, 0.0, 0.0, xpos,     ypos, 0.0, 1.0, xpos + w, ypos,     1.0, 1.0,
                            xpos, ypos + h, 0.0, 0.0, xpos + w, ypos, 1.0, 1.0, xpos + w, ypos + h, 1.0, 0.0};
            _M_mesh.update_data(0, _M_mesh.data.size() * sizeof(float));

            // Render glyph Fonture over quad
            ch.gliph->bind();
            _M_mesh.draw(Primitive::Triangle, 6, 0);
            x += (ch.advance >> 6) * scale;// Bitshift by 6 to get value in pixels (2^6 = 64)
        }
        return *this;
    }


    Font& Font::draw(const std::string& Font, const Size2D& pos, float scale)
    {
        return draw(Font, pos.x, pos.y, scale);
    }

    Font& Font::draw(const std::wstring& Font, const Size2D& pos, float scale)
    {
        return draw(Font, pos.x, pos.y, scale);
    }

    const Size2D& Font::font_size() const
    {
        return _M_size;
    }

    Font& Font::font_size(const Size2D& size)
    {
        return load(_M_font_path, size);
    }

    const std::string& Font::font_path() const
    {
        return _M_font_path;
    }

    Texture2D* Font::texture_of(char ch)
    {
        return texture_of(static_cast<wchar_t>(ch));
    }

    Texture2D* Font::texture_of(wchar_t ch)
    {
        push_char(ch);
        try
        {
            return _M_characters.at(ch).gliph;
        }
        catch (...)
        {
            return Object::new_instance<Texture2D>();
        }
    }

}// namespace Engine
