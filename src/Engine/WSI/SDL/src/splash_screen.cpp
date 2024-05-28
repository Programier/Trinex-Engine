#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Engine/font.hpp>
#include <Engine/splash_config.hpp>
#include <Image/image.hpp>
#include <window_manager.hpp>

namespace Engine
{
    static Font m_splash_font;
    static SDL_Window* m_splash_window     = nullptr;
    static SDL_Renderer* m_splash_renderer = nullptr;
    static SDL_Texture* m_splash_texture   = nullptr;
    static int_t m_splash_width            = 0;
    static int_t m_splash_height           = 0;
    static SplashConfig m_splash_config;

    static DestroyController destroy_controller([]() { m_splash_font.close(); });

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

    static uint_t font_size_of(SplashTextType type)
    {
        switch (type)
        {
            case SplashTextType::StartupProgress:
                return m_splash_config.startup_text_size;
            case SplashTextType::VersionInfo:
                return m_splash_config.version_text_size;
            case SplashTextType::CopyrightInfo:
                return m_splash_config.copyright_text_size;
            case SplashTextType::GameName:
                return m_splash_config.game_name_text_size;

            default:
                return 10.f;
        }
    }

    struct TextInfo {
        SDL_Texture* m_text_texture = nullptr;
        SDL_Rect rect;

        void reset()
        {
            if (m_text_texture)
            {
                SDL_DestroyTexture(m_text_texture);
                m_text_texture = nullptr;
            }
        }

        void init(const Image& image)
        {
            int sdl_format = to_sdl_color_format(image.format());
            if (sdl_format == SDL_PIXELFORMAT_UNKNOWN)
            {
                return;
            }

            m_text_texture = SDL_CreateTexture(m_splash_renderer, sdl_format, SDL_TEXTUREACCESS_STATIC,
                                               static_cast<int>(image.width()), static_cast<int>(image.height()));

            if (m_splash_texture == nullptr)
            {
                return;
            }

            if (SDL_UpdateTexture(m_text_texture, NULL, image.data(), static_cast<int>(image.channels() * image.width())) < 0)
            {
                reset();
                return;
            }

            SDL_SetTextureBlendMode(m_text_texture, SDL_BLENDMODE_BLEND);

            rect.w = image.width();
            rect.h = image.height();
        }
    };

    static TextInfo m_text_info[static_cast<size_t>(SplashTextType::Count)];

    static TextInfo& info_of(SplashTextType type)
    {
        return m_text_info[static_cast<size_t>(type)];
    }

    bool SDL2_WindowManagerInterface::show_splash_screen(const class Image& image, Size2D splash_size, const SplashConfig& config)
    {
        if (m_splash_window)
            return false;

        m_splash_config = config;

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

        SDL_SetRenderDrawBlendMode(m_splash_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(m_splash_texture, SDL_BLENDMODE_BLEND);

        m_splash_font.load(config.font_path);

        // Setup positions of text

        TextInfo& startup_info = info_of(SplashTextType::StartupProgress);
        startup_info.rect.x = 10;
        startup_info.rect.y = m_splash_height - font_size_of(SplashTextType::StartupProgress) - 10.f;

        TextInfo& version_info = info_of(SplashTextType::VersionInfo);
        version_info.rect.x = 10;
        version_info.rect.y = startup_info.rect.y - font_size_of(SplashTextType::VersionInfo) - 5.f;

        TextInfo& game_name_info = info_of(SplashTextType::GameName);
        game_name_info.rect.x = 10;
        game_name_info.rect.y = version_info.rect.y - font_size_of(SplashTextType::GameName) - 5.f;

        SDL_SetRenderDrawColor(m_splash_renderer, 120, 120, 120, 120);
        return true;
    }

    WindowManagerInterface& SDL2_WindowManagerInterface::update_splash_screen()
    {
        if (!m_splash_window)
            return *this;

        SDL_RenderClear(m_splash_renderer);
        SDL_RenderCopyEx(m_splash_renderer, m_splash_texture, NULL, nullptr, 0.f, nullptr, SDL_FLIP_VERTICAL);

        for (auto& image : m_text_info)
        {
            if (image.m_text_texture == nullptr)
                continue;

            // Render Shadow
            SDL_RenderFillRect(m_splash_renderer, &image.rect);

            // Render Text
            SDL_RenderCopyEx(m_splash_renderer, image.m_text_texture, nullptr, &image.rect, 0.f, nullptr, SDL_FLIP_VERTICAL);
        }

        SDL_RenderPresent(m_splash_renderer);

        return *this;
    }

    WindowManagerInterface& SDL2_WindowManagerInterface::hide_splash_screen()
    {
        m_splash_font.close();

        for (auto& text : m_text_info)
        {
            text.reset();
        }

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

    WindowManagerInterface& SDL2_WindowManagerInterface::update_splash_screen_text(SplashTextType type, const StringView& text)
    {
        if (m_splash_window == nullptr || !m_splash_font.is_valid())
            return *this;

        TextInfo& info = info_of(type);
        info.reset();

        FontConfig config;
        config.color        = Constants::splash_text_color;
        config.dynamic_size = true;
        config.font_size    = {0, font_size_of(type)};

        Image image = m_splash_font.render(text, &config);
        info.init(image);
        return *this;
    }
}// namespace Engine
