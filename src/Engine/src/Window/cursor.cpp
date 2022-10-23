#include <Core/logger.hpp>
#include <SDL.h>
#include <Window/cursor.hpp>
#include <sdl_surface.hpp>
#include <stb_image.h>


namespace Engine
{
    static void delete_cursor(void* cursor)
    {
        logger->log("Cursor: Destroy cursor: %p\n", cursor);
        if (cursor)
            SDL_FreeCursor(static_cast<SDL_Cursor*>(cursor));
    }

    static void delete_surface(void* surface)
    {
        logger->log("Cursor: Destroy surface: %p\n", surface);
        if (surface)
            SDL_FreeSurface(static_cast<SDL_Surface*>(surface));
    }

    void Cursor::update_cursor(bool only_cursor)
    {
        if (!only_cursor)
        {
            _M_SDL_surface = SmartPointer<void>(create_sdl_surface(*_M_image.get()), delete_surface);
        }

        _M_SDL_cursor = SmartPointer<void>(
                SDL_CreateColorCursor(static_cast<SDL_Surface*>(_M_SDL_surface.get()), _M_x_hotspot, _M_y_hotspot),
                delete_cursor);
        if (!_M_SDL_cursor.get())
            logger->log("Cursor: Failed to load cursor\n");
    }

    Cursor::Cursor() : _M_SDL_cursor(nullptr)
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
          _M_SDL_cursor(cursor._M_SDL_cursor), _M_SDL_surface(cursor._M_SDL_surface)
    {}

    Cursor& Cursor::operator=(const Cursor& cursor)
    {
        if (this == &cursor)
            return *this;
        _M_image = cursor._M_image;
        _M_x_hotspot = cursor._M_x_hotspot;
        _M_y_hotspot = cursor._M_y_hotspot;
        _M_SDL_cursor = cursor._M_SDL_cursor;
        _M_SDL_surface = cursor._M_SDL_surface;
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

    void* Cursor::sdl_cursor() const
    {
        return _M_SDL_cursor.get();
    }

    Cursor::~Cursor()
    {
        _M_SDL_cursor = nullptr;
        _M_SDL_surface = nullptr;
    }

}// namespace Engine
