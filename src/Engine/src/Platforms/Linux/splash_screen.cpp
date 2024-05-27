#include <Core/config_manager.hpp>
#include <Core/thread.hpp>
#include <Image/image.hpp>
#include <Platform/platform.hpp>
#include <Window/monitor.hpp>
#include <Window/window_manager.hpp>

namespace Engine::Platform
{
    static Thread* m_splash_screen_thread = nullptr;
    static Atomic<bool> m_splash_active   = false;

    static void splash_main(const Image& splash_image, Size2D splash_size)
    {
        WindowManager::instance()->show_splash_screen(splash_image, splash_size);

        while (m_splash_active)
        {
            WindowManager::instance()->update_splash_screen();
            Thread::sleep_for(0.033f);
        }

        WindowManager::instance()->hide_splash_screen();
    }

    ENGINE_EXPORT void show_splash_screen(const StringView& splash_name)
    {
        if (m_splash_screen_thread)
            return;

        Image image;
        Path path = ConfigManager::get_path("Engine::splash_dir") / splash_name;
        image.load(path);

        if (image.empty())
            return;

        m_splash_screen_thread = new Thread("Splash");

        struct ShowSplash : public ExecutableObject {
            Image m_image;
            Size2D m_size;

            ShowSplash(const Image& image, Size2D splash_size) : m_image(image), m_size(splash_size)
            {}

            int_t execute() override
            {
                splash_main(m_image, m_size);
                return sizeof(ShowSplash);
            }
        };

        m_splash_active = true;

        Size2D splash_size = image.size();
        splash_size        = (splash_size / splash_size.x) * static_cast<float>(Monitor::width()) / 3.f;
        m_splash_screen_thread->insert_new_task<ShowSplash>(image, splash_size);
    }

    ENGINE_EXPORT void hide_splash_screen()
    {
        if (!m_splash_screen_thread)
            return;
        m_splash_active = false;

        m_splash_screen_thread->wait_all();
        delete m_splash_screen_thread;
    }
}// namespace Engine::Platform
