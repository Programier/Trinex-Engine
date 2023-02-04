#include <Core/application.hpp>
#include <Core/benchmark.hpp>
#include <Core/engine.hpp>
#include <Core/file_reader.hpp>
#include <Core/logger.hpp>
#include <Graphics/shader.hpp>
#include <Window/window.hpp>
#include <api.hpp>
#include <thread>


namespace Engine
{

    Application::Application()
    {}

    Application& Application::init()
    {
        if (EngineInstance::instance())
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
        window.background_color(Color::Black);

        ShaderParams params;
        FileReader reader;

        params.name = "main";
        reader.open("/home/programier/Projects/Shaders/frag.spv").read(params.binaries.fragment);
        reader.open("/home/programier/Projects/Shaders/vert.spv").read(params.binaries.vertex);

        reader.close();


        struct Value {
            Vector3D color;
        };


        struct Value2 {
            Vector2D position;
        };

        Value value1, value2, value3;

        value1.color = Color::Red;
        value2.color = Color::Crimson;
        value3.color = Color::Yellow;

        ShaderUniformVariable var;
        var.name = "ubo";
        var.binding = 0;
        var.size = sizeof(Value);

        params.uniform_variables.push_back(var);

        var.name = "ubo2";
        var.binding = 1;
        params.uniform_variables.push_back(var);

        var.name = "ubo3";
        var.binding = 2;
        params.uniform_variables.push_back(var);


        Shader shader(params);


        while (window.is_open())
        {
            EngineInstance::_M_instance->api_interface()->begin_render();
            window.bind().clear_buffer();

            EngineInstance::_M_instance->api_interface()->begin_render_pass();
            shader.use().set("ubo", &value1).set("ubo2", &value2).set("ubo3", &value3);
            EngineInstance::_M_instance->api_interface()->end_render_pass();

            EngineInstance::_M_instance->api_interface()->end_render();
            window.swap_buffers();

            on_render_frame();
            _M_update_event_callback();
        }

        EngineInstance::_M_instance->_M_api_interface->wait_idle();

        return *this;
    }


    Application::~Application()
    {
        logger->log("Terminate application");
        window.destroy();

        delete EngineInstance::_M_instance;
    }

}// namespace Engine
