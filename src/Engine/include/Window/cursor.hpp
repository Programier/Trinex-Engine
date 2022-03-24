#pragma once
#include "../Image/image.hpp"


namespace Engine
{
    class Cursor
    {
        Image _M_image;
        int _M_x_hotspot;
        int _M_y_hotspot;
        void* _M_glfw_cursor;

        void update_cursor();

    public:
        Cursor();
        Cursor(const Image& image, int x_hotspot, int y_hotspot);
        Cursor(const std::string& path, int x_hotspot, int y_hotspot);
        Cursor(const Cursor&);
        Cursor& operator=(const Cursor&);

        int x_hotspot();
        int y_hotspot();
        Cursor& x_hotspot(int x);
        Cursor& y_hotspot(int x);

        const Image& image();
        Cursor& image(const Image&);

        void* glfw_cursor();
        ~Cursor();
    };

}// namespace Engine
