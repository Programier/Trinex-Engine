#include <Core/application.hpp>
#include <Core/benchmark.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Window/window.hpp>
#include <api.hpp>
#include <thread>


namespace Engine
{

    Application::Application()
    {}

    Application& Application::init()
    {
        if (EngineInstance::get_instance())
        {
            throw std::runtime_error("Cannot initialize another application!");
        }

        EngineInstance* engine = EngineInstance::create_instance();
        engine->_M_api = this->init_info.api;
        engine->init();

        // Creating window
        window.init(init_info.window_size, init_info.window_name, init_info.window_attribs);
        _M_update_event_callback = Event::poll_events;
        return on_init();
    }

    Application& Application::on_init()
    {
        return *this;
    }

    Application& Application::update_logic()
    {
        return *this;
    }

    Application& Application::on_render_frame()
    {
        return *this;
    }

    Application& Application::start()
    {
        window.background_color(Color::MediumOrchid);

        while (window.is_open())
        {
            EngineInstance::_M_instance->api_interface()->begin_render();

            window.bind().clear_buffer();

            EngineInstance::_M_instance->api_interface()->begin_render_pass();
            EngineInstance::_M_instance->api_interface()->end_render_pass();
            EngineInstance::_M_instance->api_interface()->end_render();
            window.swap_buffers();

            on_render_frame();
            _M_update_event_callback();
        }


        return *this;
    }


    Application::~Application()
    {
        logger->log("Terminate application");
        window.close();

        delete EngineInstance::get_instance();
    }

}// namespace Engine
