#include "cursor.hpp"
#include <GLFW/glfw3.h>
#include <iostream>


namespace Engine
{
    void Cursor::update_cursor()
    {
        if (_M_glfw_cursor != nullptr)
        {
            glfwDestroyCursor(static_cast<GLFWcursor*>(_M_glfw_cursor));
        }
        _M_image.add_alpha_channel();
        if (_M_image.channels() == 0 || _M_image.width() == 0 || _M_image.height() == 0 ||
            _M_image.glfw_image() == nullptr)
            return;
        _M_glfw_cursor = static_cast<void*>(glfwCreateCursor(
                static_cast<GLFWimage*>(_M_image.glfw_image()), _M_x_hotspot, _M_y_hotspot));
    }

    Cursor::Cursor() : _M_glfw_cursor(nullptr)
    {}

    Cursor::Cursor(const Image& image, int x_hotspot, int y_hotspot)
        : _M_image(image), _M_x_hotspot(x_hotspot), _M_y_hotspot(y_hotspot)
    {
        update_cursor();
    }

    Cursor::Cursor(const std::string& path, int x_hotspot, int y_hotspot)
        : _M_image(path), _M_x_hotspot(x_hotspot), _M_y_hotspot(y_hotspot)
    {
        update_cursor();
    }

    Cursor::Cursor(const Cursor& cursor)
        : _M_image(cursor._M_image), _M_x_hotspot(cursor._M_x_hotspot),
          _M_y_hotspot(cursor._M_y_hotspot)
    {
        update_cursor();
    }

    Cursor& Cursor::operator=(const Cursor& cursor)
    {
        if (this == &cursor)
            return *this;
        _M_image = cursor._M_image;
        _M_x_hotspot = cursor._M_x_hotspot;
        _M_y_hotspot = cursor._M_y_hotspot;
        update_cursor();
        return *this;
    }

    int Cursor::x_hotspot()
    {
        return _M_x_hotspot;
    }

    int Cursor::y_hotspot()
    {
        return _M_y_hotspot;
    }

    Cursor& Cursor::x_hotspot(int x)
    {
        _M_x_hotspot = x;
        update_cursor();
        return *this;
    }

    Cursor& Cursor::y_hotspot(int y)
    {
        _M_y_hotspot = y;
        update_cursor();
        return *this;
    }


    const Image& Cursor::image()
    {
        return _M_image;
    }

    Cursor& Cursor::image(const Image& img)
    {
        _M_image = img;
        update_cursor();
        return *this;
    }

    void* Cursor::glfw_cursor()
    {
        return _M_glfw_cursor;
    }

    Cursor::~Cursor()
    {
        if (_M_glfw_cursor)
        {
            glfwDestroyCursor(static_cast<GLFWcursor*>(_M_glfw_cursor));
            _M_glfw_cursor = nullptr;
        }
    }

}// namespace Engine
