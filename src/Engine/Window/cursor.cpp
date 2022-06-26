#include <GLFW/glfw3.h>
#include <Window/cursor.hpp>
#include <iostream>


namespace Engine
{

    static void delete_cursor(void* address)
    {
        std::clog << "Cursor: Destroy cursor: " << address << std::endl;
        glfwDestroyCursor(reinterpret_cast<GLFWcursor*>(address));
    }

    void Cursor::update_cursor()
    {
        _M_glfw_cursor = nullptr;


        if (_M_image.get() && !(*_M_image).empty())
        {
            (*_M_image).add_alpha_channel();
            if ((*_M_image).channels() == 0 || (*_M_image).width() == 0 || (*_M_image).height() == 0 ||
                (*_M_image).glfw_image() == nullptr)
                return;

            GLFWimage* img = reinterpret_cast<GLFWimage*>((*_M_image).glfw_image());
            _M_glfw_cursor = SmartPointer<void>(reinterpret_cast<void*>(glfwCreateCursor(img, _M_x_hotspot, _M_y_hotspot)),
                                                delete_cursor);
            std::clog << "Cursor: Created cursor: " << _M_glfw_cursor.get() << std::endl;
        }
    }

    Cursor::Cursor() : _M_glfw_cursor(nullptr)
    {}

    Cursor::Cursor(const Image& image, int x_hotspot, int y_hotspot)
        : _M_image(SmartPointer<Image>(new Image(image), delete_value<Image>)), _M_x_hotspot(x_hotspot),
          _M_y_hotspot(y_hotspot)
    {
        update_cursor();
    }

    Cursor::Cursor(const std::string& path, int x_hotspot, int y_hotspot)
        : _M_image(SmartPointer<Image>(new Image(path), delete_value<Image>)), _M_x_hotspot(x_hotspot),
          _M_y_hotspot(y_hotspot)
    {
        update_cursor();
    }

    Cursor::Cursor(const Cursor& cursor)
        : _M_image(cursor._M_image), _M_x_hotspot(cursor._M_x_hotspot), _M_y_hotspot(cursor._M_y_hotspot),
          _M_glfw_cursor(cursor._M_glfw_cursor)
    {}

    Cursor& Cursor::operator=(const Cursor& cursor)
    {
        if (this == &cursor)
            return *this;
        _M_image = cursor._M_image;
        _M_x_hotspot = cursor._M_x_hotspot;
        _M_y_hotspot = cursor._M_y_hotspot;
        _M_glfw_cursor = cursor._M_glfw_cursor;
        return *this;
    }

    int Cursor::x_hotspot() const
    {
        return _M_x_hotspot;
    }

    int Cursor::y_hotspot() const
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


    const Image& Cursor::image() const
    {
        return *_M_image;
    }

    Cursor& Cursor::image(const Image& img)
    {
        _M_image = SmartPointer<Image>(new Image(img), delete_value<Image>);
        update_cursor();
        return *this;
    }

    void* Cursor::glfw_cursor() const
    {
        return _M_glfw_cursor.get();
    }

    Cursor::~Cursor()
    {
        _M_glfw_cursor = nullptr;
    }

}// namespace Engine
