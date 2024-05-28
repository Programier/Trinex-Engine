#include <Core/config_manager.hpp>
#include <Core/thread.hpp>
#include <Engine/font.hpp>
#include <Engine/splash_config.hpp>
#include <Image/image.hpp>
#include <Platform/platform.hpp>
#include <Window/monitor.hpp>
#include <Window/window_manager.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine::Platform
{
    static Atomic<bool> m_splash_active = false;

    static Thread* m_splash_screen_exec_thread = nullptr;
    static Thread* m_splash_screen_main_thread = nullptr;

    class ShowSplash : public ExecutableObject
    {
        const SplashConfig& m_config;
        const Image& m_splash_image;
        Size2D m_splash_size;

    public:
        ShowSplash(const SplashConfig& config, const Image& splash_image, Size2D splash_size)
            : m_config(config), m_splash_image(splash_image), m_splash_size(splash_size)
        {}

        int_t execute() override
        {
            WindowManager::instance()->show_splash_screen(m_splash_image, m_splash_size, m_config);
            return sizeof(ShowSplash);
        }
    };

    class HideSplash : public ExecutableObject
    {
    public:
        int_t execute() override
        {
            WindowManager::instance()->hide_splash_screen();
            return sizeof(HideSplash);
        }
    };


    class UpdateSplash : public ExecutableObject
    {
    public:
        int_t execute() override
        {
            WindowManager::instance()->update_splash_screen();
            return sizeof(UpdateSplash);
        }
    };

    class UpdateSplashText : public ExecutableObject
    {
        SplashTextType m_type;
        String m_text;

    public:
        UpdateSplashText(SplashTextType type, const StringView& text) : m_type(type), m_text(text)
        {}

        int_t execute() override
        {
            WindowManager::instance()->update_splash_screen_text(m_type, m_text);
            return sizeof(UpdateSplashText);
        }
    };

    static void splash_main(const SplashConfig& config, const Image& splash_image, Size2D splash_size)
    {
        m_splash_screen_exec_thread->insert_new_task<ShowSplash>(config, splash_image, splash_size);
        m_splash_active = true;

        while (m_splash_active)
        {
            m_splash_screen_exec_thread->insert_new_task<UpdateSplash>();
            m_splash_screen_exec_thread->wait_all();

            Thread::sleep_for(0.033f);
        }

        m_splash_screen_exec_thread->insert_new_task<HideSplash>();
    }

    ENGINE_EXPORT void show_splash_screen()
    {
        if (m_splash_screen_main_thread)
            return;

        SplashConfig config;

        Image image(config.image_path);

        if (image.empty())
            return;

        m_splash_screen_main_thread = new Thread("Splash");
        m_splash_screen_exec_thread = new Thread("Splash Exec");

        struct SplashMain : public ExecutableObject {
            Image m_image;
            Size2D m_size;
            SplashConfig m_config;


            SplashMain(const SplashConfig& config, const Image& image, Size2D splash_size)
                : m_image(image), m_size(splash_size), m_config(config)
            {}

            int_t execute() override
            {
                splash_main(m_config, m_image, m_size);
                return sizeof(SplashMain);
            }
        };

        m_splash_active    = false;
        Size2D splash_size = image.size();
        splash_size        = (splash_size / splash_size.x) * static_cast<float>(Monitor::width()) / 3.f;
        m_splash_screen_main_thread->insert_new_task<SplashMain>(config, image, splash_size);

        while (!m_splash_active)
        {
            Thread::sleep_for(0.001);
        }
    }

    ENGINE_EXPORT void splash_screen_text(SplashTextType type, const StringView& text)
    {
        if (m_splash_screen_main_thread != nullptr)
            m_splash_screen_exec_thread->insert_new_task<UpdateSplashText>(type, text);
    }

    ENGINE_EXPORT void hide_splash_screen()
    {
        if (!m_splash_screen_main_thread)
            return;

        m_splash_active = false;

        m_splash_screen_main_thread->wait_all();
        m_splash_screen_exec_thread->wait_all();

        delete m_splash_screen_exec_thread;
        delete m_splash_screen_main_thread;

        m_splash_screen_main_thread = nullptr;
        m_splash_screen_exec_thread = nullptr;
    }
}// namespace Engine::Platform
