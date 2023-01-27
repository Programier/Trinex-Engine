#pragma once
#include "../Image/image.hpp"
#include <TemplateFunctional/smart_pointer.hpp>
#include <Core/export.hpp>


namespace Engine
{
    CLASS Cursor
    {
        SmartPointer<Image> _M_image = nullptr;;
        int_t _M_x_hotspot;
        int_t _M_y_hotspot;
        SmartPointer<void> _M_SDL_cursor;
        SmartPointer<void> _M_SDL_surface;

        void update_cursor(bool only_cursor = false);

    public:
        Cursor();
        Cursor(const Image& image, int_t x_hotspot, int_t y_hotspot);
        Cursor(const String& path, int_t x_hotspot, int_t y_hotspot);
        Cursor(const Cursor&);
        Cursor& operator=(const Cursor&);

        int_t x_hotspot() const;
        int_t y_hotspot() const;
        Cursor& x_hotspot(int_t x);
        Cursor& y_hotspot(int_t x);

        const Image& image() const;
        Cursor& image(const Image&);

        void* sdl_cursor() const;
        ~Cursor();
    };

}// namespace Engine
