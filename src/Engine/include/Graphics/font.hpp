#pragma once
#include <Core/engine_types.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/texture_2D.hpp>
#include <TemplateFunctional/smart_pointer.hpp>
#include <map>
#include <string>

namespace Engine
{
    CLASS Font : public Object
    {
        struct Character {
            Texture2D* gliph;
            glm::ivec2 size;
            glm::ivec2 bearing;
            unsigned int advance;
        };

        std::map<unsigned long, Character> _M_characters;
        SmartPointer<void> _M_ft = nullptr;
        SmartPointer<void> _M_face = nullptr;
        Size2D _M_size;
        std::string _M_font_path;
        Mesh<float> _M_mesh;

        Font& terminate();
        Font& push_char(wchar_t ch);


        declare_instance_info_hpp(Font);
    public:
        constructor_hpp(Font);
        constructor_hpp(Font, const std::string& file, const Size2D& size);

        Font& draw(const std::string& Font, Size1D x, Size1D y, float scale = 1.f);
        Font& draw(const std::string& Font, const Size2D& pos, float scale = 1.f);
        Font& draw(const std::wstring& Font, Size1D x, Size1D y, float scale = 1.f);
        Font& draw(const std::wstring& Font, const Size2D& pos, float scale = 1.f);
        Texture2D* texture_of(char ch);
        Texture2D* texture_of(wchar_t ch);
        const Size2D& font_size() const;
        Font& font_size(const Size2D& size);
        const std::string& font_path() const;
        Font& load(const std::string& path, const Size2D& size);
        ~Font();
    };
}// namespace Engine
