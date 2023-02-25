#include <Core/application.hpp>
#include <Core/benchmark.hpp>
#include <Core/engine.hpp>
#include <Core/file_reader.hpp>
#include <Core/logger.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture_2D.hpp>
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
        _M_update_event_callback = Event::wait_for_event;
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

        struct Vertex {
            Vector2D pos;
            Vector2D coord;
        };

        params.vertex_info.size = sizeof(Vertex);
        params.vertex_info.binding = 0;
        VertexAtribute attrib;

        attrib.offset = offsetof(Vertex, pos);
        attrib.type = ShaderDataType::type_of<Vector2D>();
        params.vertex_info.attributes.push_back(attrib);


        attrib.offset = offsetof(Vertex, coord);
        attrib.type = ShaderDataType::type_of<Vector2D>();
        params.vertex_info.attributes.push_back(attrib);

        ShaderTextureSampler sampler;
        sampler.binding = 1;

        params.texture_samplers.push_back(sampler);

        FileReader reader;


        params.name = "main";
        reader.open("/home/programier/Projects/Shaders/frag.spv").read(params.binaries.fragment);
        reader.open("/home/programier/Projects/Shaders/vert.spv").read(params.binaries.vertex);


        params.text.fragment.emplace_back();
        params.text.vertex.emplace_back();

        reader.open("/home/programier/Projects/Shaders/frag.frag").read(params.text.fragment[0]);
        reader.open("/home/programier/Projects/Shaders/vert.vert").read(params.text.vertex[0]);

        reader.close();

        Shader shader(params);

        Mesh<Vertex>* mesh = Object::new_instance<Mesh<Vertex>>();

        //vec2(0.0, -0.5),
        //vec2(0.5, 0.5),
        //vec2(-0.5, 0.5)


        Vertex vertex;
        vertex.pos = {-1, -1};
        vertex.coord = {0, 0};

        mesh->data.push_back(vertex);

        vertex.pos = {-1, 1};
        vertex.coord = {0, 1};
        mesh->data.push_back(vertex);

        vertex.pos = {1, 1};
        vertex.coord = {1, 1};
        mesh->data.push_back(vertex);

        vertex.pos = {1, -1};
        vertex.coord = {1, 0};
        mesh->data.push_back(vertex);

        mesh->indexes = {0, 1, 2, 0, 2, 3};

        mesh->gen();
        mesh->set_data().set_indexes();

        Texture2D texture;
        texture.load("/home/programier/Projects/icon.jpg");

        while (window.is_open())
        {
            EngineInstance::_M_instance->api_interface()->begin_render();
            window.bind().clear_buffer();

            EngineInstance::_M_instance->api_interface()->begin_render_pass();

            shader.use();
            texture.bind(1);
            mesh->draw(Primitive::Triangle, mesh->indexes_size(), 0);

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
        EngineInstance::_M_instance->trigger_terminate_functions();

        window.destroy();
        delete EngineInstance::_M_instance;
    }

}// namespace Engine
