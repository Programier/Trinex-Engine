#include <Graphics/camera.hpp>
#include <Graphics/enable_param.hpp>
#include <Graphics/light.hpp>
#include <Graphics/line.hpp>
#include <Graphics/shader_system.hpp>
#include <Graphics/skybox.hpp>
#include <Graphics/terrainmodel.hpp>
#include <Graphics/text.hpp>
#include <Init/init.hpp>
#include <Window/color.hpp>
#include <Window/window.hpp>
#include <engine.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <list>
#include <sstream>


using namespace Engine;
#define cmd [](std::wstringstream & stream, Program & p)


struct Program {

    Window window;

    Cursor cursor;

    Text text_renderer;
    std::list<std::wstring> window_log;

    TerrainModel scene;
    glm::mat4 scene_matrix;
    Line scene_polygones;

    Camera main_camera = Camera({0, 0, 0}, glm::radians(90.f));
    Camera* camera = &main_camera;
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 projview;
    Image icon;

    Skybox skybox;

    int current_fps = 60;
    Key current_key;
    std::wstring string_fps;
    std::size_t frame = 0;


    float mouse_sensitive = 10;
    bool active_console = false;

    float speed = 15;
    float speed_k = 1.f;
    float diff_time;
    float line_width = 1.5f;

    static constexpr float angle_limit = glm::radians(89.f);

    std::wstring current_command;

    std::map<std::wstring, std::function<void(std::wstringstream& stream, Program& p)>> command_map;

    enum RenderType : int
    {
        Scene = 0,
        Polygones = 1,
        All = 2
    } Render = RenderType::Scene;

    Light light;
    Light::Material material;
    Camera light_camera = Camera({0, 0, 0}, glm::radians(90.f));
    bool lighting = true;
    glm::mat4 light_projview;

    bool depth_rendering = true;
    Mesh depth_mesh;
    glm::mat3 transposed_inversed_model;

    // Methods

    Program()
        : window(1280, 720, "Scene", true), cursor("resources/cursor.png", 0, 0),
          text_renderer("resources/fonts/font.otf", {0.f, 100.f}), icon("resources/icon.jpg")
    {
        icon.add_alpha_channel();
        window.cursor(cursor).icon(icon);
        // Run programm
        init_map();
        init_shader();
        load_resources();
        enable(EnableCap::Blend)(EnableCap::DepthTest);
        blend_func(BlendFunc::SrcAlpha, BlendFunc::OneMinusSrcAlpha);
        camera->max_render_distance(200.f);
        //window.vsync(false);
        current_command.reserve(50);


        material.shininess = 32.f;
        material.ambient = material.specular = LightColor(0.3f);
        material.diffuse = LightColor(1.f);

        light.type = LightType::Point;
        light.specular = light.ambient = material.ambient;
        light.diffuse = material.diffuse;
        light.buffer.gen(Monitor::size());


        while (window.is_open())
        {
            window.event.poll_events();
            window.clear_buffer();
            diff_time = window.event.diff_time();
            current_key = window.event.keyboard.just_pressed();

            if (frame % 30 == 0)
                string_fps = std::to_wstring((current_fps = static_cast<int>(1.f / window.event.diff_time())));

            if (!active_console)
            {
                camera_proccess();
                control();
            }

            proj = main_camera.projection(window);
            view = main_camera.view();
            projview = proj * view;
            if (lighting)
            {
                light.buffer.bind();
                light.buffer.bind_texture();
                light.buffer.view_port({0, 0}, Monitor::size()).clear_buffer();
                light_render();

                window.bind().update_view_port();
            }


            render_scene();

            if (depth_rendering)
            {
                ShaderSystem::DepthRenderer::shader.use();
                light.buffer.bind_texture(0);

                depth_mesh.draw(Primitive::TRIANGLE);
            }

            update_text().render_text();
            frame++;
            window.swap_buffers();
        }
    }


    void init_map()
    {
        command_map[L"vsync"] = cmd
        {
            bool value;
            stream >> value;
            p.window.vsync(value);
        };

        command_map[L"mouse_sensitive"] = cmd
        {
            float value;
            stream >> value;
            p.mouse_sensitive = value;
        };

        command_map[L"speed"] = cmd
        {
            float value;
            stream >> value;
            p.speed = value;
        };

        command_map[L"line_width"] = cmd
        {
            float value;
            stream >> value;
            p.line_width = value;
        };

        command_map[L"min_render"] = cmd
        {
            float value;
            stream >> value;
            p.camera->min_render_distance(std::abs(value));
        };

        command_map[L"max_render"] = cmd
        {
            float value;
            stream >> value;
            p.camera->max_render_distance(std::abs(value));
        };

        command_map[L"fovy"] = cmd
        {
            float value;
            stream >> value;
            p.camera->viewing_angle(std::abs(glm::radians(value)));
        };

        command_map[L"cam"] = cmd
        {
            int value;
            stream >> value;
            static Camera* cameras[] = {&p.main_camera, &p.light_camera};
            p.camera = cameras[glm::abs(value) % (sizeof(cameras) / sizeof(Camera*))];
        };

        command_map[L"lighting"] = cmd
        {
            stream >> p.lighting;
        };

        command_map[L"depth_rendering"] = cmd
        {
            stream >> p.depth_rendering;
        };
    }

    void parse_command()
    {
        std::wstringstream stream(current_command);
        std::wstring command;
        stream >> command;
        try
        {
            command_map.at(command)(stream, *this);
        }
        catch (...)
        {}

        current_command.clear();
    }


    void console()
    {
        if (current_key == KEY_TAB && (active_console = !active_console))
        {
            window.event.keyboard.last_symbol();
            return current_command.clear();
        }

        if (!active_console)
            return;

        static const std::wstring arrow = L"--> ";
        unsigned int last_symbol = window.event.keyboard.last_symbol();
        if (last_symbol)
            current_command.push_back(last_symbol);

        if (current_key == Key::KEY_BACKSPACE && !current_command.empty())
            current_command.pop_back();

        if (current_key == Key::KEY_ENTER)
            parse_command();


        window_log.push_back(arrow + current_command);
    }

    Program& update_text()
    {
        window_log.clear();
        window_log.push_back(std::wstring(L"FPS: ") + string_fps);
        console();
        return *this;
    }

    Program& render_text()
    {
        float current_pos = window.height();

        ShaderSystem::Text::shader.use()
                .set(ShaderSystem::Text::projview, glm::ortho(0.f, window.width(), 0.f, window.height(), 0.f, 1.f))
                .set(ShaderSystem::Text::color, glm::vec4(White));

        for (auto& line : window_log)
        {
            current_pos -= (text_renderer.font_size().y * 0.2f + 2);
            text_renderer.draw(line, 10, current_pos, 0.2f);
        }

        return *this;
    }

    void render_scene_textured()
    {
        ShaderSystem::Scene::shader.use()
                .set(ShaderSystem::Scene::lighting, lighting)
                .set(ShaderSystem::Scene::camera_pos, main_camera.position())
                .set(ShaderSystem::Scene::model, scene_matrix)
                .set(ShaderSystem::Scene::projview, projview)
                .set(ShaderSystem::Scene::light_projview, light_projview)
                .set(ShaderSystem::Scene::transposed_inversed_model, transposed_inversed_model)
                .set("texture1", 1);
        light.buffer.bind_texture(1);
        light.send_to_shader(ShaderSystem::Scene::shader, "light");
        material.send_to_shader(ShaderSystem::Scene::shader, "material");

        scene.draw();
    }

    void light_render()
    {
        light_projview = light_camera.projection(Monitor::size()) * light_camera.view();
        light.position = light_camera.position();
        light.direction = -light_camera.front_vector();
        ShaderSystem::Depth::shader.use()
                .set(ShaderSystem::Depth::model, scene_matrix)
                .set(ShaderSystem::Depth::projview, light_projview);
        scene.draw();
    }

    void render_scene()
    {
        if (Render == RenderType::Scene)
            render_scene_textured();
        else if (Render == RenderType::Polygones)
            render_scene_polygones();
        else
        {
            render_scene_textured();
            render_scene_polygones();
        }
        render_skybox();
    }

    void render_scene_polygones()
    {
        ShaderSystem::Line::shader.use()
                .set(ShaderSystem::Line::projview, projview)
                .set(ShaderSystem::Line::model, scene_matrix)
                .set(ShaderSystem::Line::color, Red);
        scene_polygones.line_width(line_width).draw();
        render_skybox();
    }


    void render_skybox()
    {
        ShaderSystem::SkyBox::shader.use().set(ShaderSystem::SkyBox::projview, proj * glm::mat4(glm::mat3(view)));
        skybox.draw();
    }

    void load_resources()
    {
        skybox.load("resources/skybox/skybox.png");
        scene.load_model("resources/scene/scene.gltf", DrawMode::LINEAR, 90);
        scene_matrix = scene.model();
        transposed_inversed_model = glm::transpose(glm::inverse(glm::mat3(scene_matrix)));
        scene_polygones.lines_from(scene);

        depth_mesh.data() = {0.5,  0.5,  0, 0, 0, 0.5, 1.f, 0.f, 0.f, 1.f, 1, 1,   0, 1, 1,
                             0.5f, 0.5f, 0, 0, 0, 1,   1,   0,   1,   1,   1, 0.5, 0, 1, 0};
        depth_mesh.attributes({3, 2}).vertices_count(6).update_buffers();
    }


    void control()
    {
        switch (current_key)
        {

            case KEY_H:
                window.cursor_mode(window.cursor_mode() == CursorMode::DISABLED ? CursorMode::NORMAL : CursorMode::DISABLED);
                break;

            case KEY_F:
                window.mode(window.mode() == WindowMode::FULLSCREEN ? WindowMode::NONE : WindowMode::FULLSCREEN);
                break;
            case KEY_0:
                Render = RenderType::Scene;
                break;
            case KEY_1:
                Render = RenderType::Polygones;
                break;

            case KEY_2:
                Render = RenderType::All;
                break;

            case KEY_R:
                ShaderSystem::init();
                break;


            default:
                break;
        }
    }

    void camera_proccess()
    {
        if (window.cursor_mode() == CursorMode::DISABLED)
        {
            auto offset = (mouse_sensitive * window.event.mouse.offset() / (window.size())) * speed * diff_time;
            float angle = Engine::angle_between(camera->up_vector(), Engine::OY) * (camera->front_vector()[1] > 0 ? 1 : -1);

            if (glm::abs(angle) <= angle_limit || angle * offset.y > 0)
                camera->rotate(offset.y, camera->right_vector());
            camera->rotate(-offset.x, OY);
        }

        if (window.event.pressed(KEY_W))
            camera->move(remove_coord(-camera->front_vector(), Coord::Y) * speed * diff_time * speed_k);

        if (window.event.pressed(KEY_S))
            camera->move(remove_coord(camera->front_vector(), Coord::Y) * speed * diff_time * speed_k);

        if (window.event.pressed(KEY_A))
            camera->move(-camera->right_vector() * speed * diff_time * speed_k);

        if (window.event.pressed(KEY_D))
            camera->move(camera->right_vector() * speed * diff_time * speed_k);

        if (window.event.pressed(KEY_UP))
            camera->move(OY * speed * diff_time * speed_k);

        if (window.event.pressed(KEY_DOWN))
            camera->move(-OY * speed * diff_time * speed_k);

        if (window.event.pressed(KEY_LEFT_SHIFT))
            speed_k = 0.1;
        else if (window.event.pressed(KEY_SPACE))
            speed_k = 10.f;
        else
            speed_k = 1.f;
    }
};


int main()
{
    Engine::init();
    Program p;
}
