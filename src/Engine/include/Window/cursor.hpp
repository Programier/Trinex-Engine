#pragma once
#include "../Image/image.hpp"
#include <BasicFunctional/smart_pointer.hpp>


namespace Engine
{
    class Cursor
    {
        SmartPointer<Image> _M_image = nullptr;;
        int _M_x_hotspot;
        int _M_y_hotspot;
        SmartPointer<void> _M_SDL_cursor;
        SmartPointer<void> _M_SDL_surface;

        void update_cursor(bool only_cursor = false);

    public:
        Cursor();
        Cursor(const Image& image, int x_hotspot, int y_hotspot);
        Cursor(const std::string& path, int x_hotspot, int y_hotspot);
        Cursor(const Cursor&);
        Cursor& operator=(const Cursor&);

        int x_hotspot() const;
        int y_hotspot() const;
        Cursor& x_hotspot(int x);
        Cursor& y_hotspot(int x);

        const Image& image() const;
        Cursor& image(const Image&);

        void* sdl_cursor() const;
        ~Cursor();
    };

}// namespace Engine
