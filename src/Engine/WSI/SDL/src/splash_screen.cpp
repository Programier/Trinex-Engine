#include <Core/logger.hpp>
#include <Image/image.hpp>
#include <window_manager.hpp>

namespace Engine
{
    static SDL_Window* m_splash_window     = nullptr;
    static SDL_Renderer* m_splash_renderer = nullptr;
    static SDL_Texture* m_splash_texture   = nullptr;
    static int_t m_splash_width            = 0;
    static int_t m_splash_height           = 0;

    static int to_sdl_color_format(ColorFormat format)
    {
        if (format == Engine::ColorFormat::R8G8B8A8)
            return SDL_PIXELFORMAT_RGBA32;

        return SDL_PIXELFORMAT_UNKNOWN;
    }

    static bool on_splash_init_fail(SDL2_WindowManagerInterface* interface)
    {
        error_log("Splash", "%s", SDL_GetError());
        interface->hide_splash_screen();
        return false;
    }

    bool SDL2_WindowManagerInterface::show_splash_screen(const class Image& image, Size2D splash_size)
    {
        if (m_splash_window)
            return false;

        m_splash_width  = static_cast<int>(splash_size.x);
        m_splash_height = static_cast<int>(splash_size.y);

        m_splash_window = SDL_CreateWindow("##SplashScreen", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_splash_width,
                                           m_splash_height, SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);

        if (m_splash_window == nullptr)
            return false;

        m_splash_renderer = SDL_CreateRenderer(m_splash_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        if (m_splash_renderer == nullptr)
        {
            return on_splash_init_fail(this);
        }

        int sdl_format = to_sdl_color_format(image.format());

        if (sdl_format == SDL_PIXELFORMAT_UNKNOWN)
        {
            return on_splash_init_fail(this);
        }

        m_splash_texture = SDL_CreateTexture(m_splash_renderer, sdl_format, SDL_TEXTUREACCESS_STATIC,
                                             static_cast<int>(image.width()), static_cast<int>(image.height()));

        if (m_splash_texture == nullptr)
        {
            return on_splash_init_fail(this);
        }

        if (SDL_UpdateTexture(m_splash_texture, NULL, image.data(), static_cast<int>(image.channels() * image.width())) < 0)
        {
            return on_splash_init_fail(this);
        }

        return true;
    }

    WindowManagerInterface& SDL2_WindowManagerInterface::update_splash_screen()
    {
        if (!m_splash_window)
            return *this;

        SDL_RenderClear(m_splash_renderer);
        SDL_Rect dst;
        dst.x = 0;
        dst.y = m_splash_height;
        dst.w = m_splash_width;
        dst.h = -m_splash_height;

        SDL_RenderCopyEx(m_splash_renderer, m_splash_texture, NULL, nullptr, 0.f, nullptr, SDL_FLIP_VERTICAL);
        SDL_RenderPresent(m_splash_renderer);

        return *this;
    }

    WindowManagerInterface& SDL2_WindowManagerInterface::hide_splash_screen()
    {
        if (m_splash_texture)
        {
            SDL_DestroyTexture(m_splash_texture);
            m_splash_texture = nullptr;
        }

        if (m_splash_renderer)
        {
            SDL_DestroyRenderer(m_splash_renderer);
            m_splash_renderer = nullptr;
        }

        if (m_splash_window)
        {
            SDL_DestroyWindow(m_splash_window);
            m_splash_window = nullptr;
        }
        return *this;
    }
}// namespace Engine
