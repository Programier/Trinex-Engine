#include <BasicFunctional/octree.hpp>
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
#include <chrono>
#include <engine.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <list>
#include <sstream>


using namespace Engine;
#define cmd [](std::wstringstream & stream, Program & p)

struct Program {

    std::list<std::wstring> prev_commands;
    std::list<std::wstring>::iterator command_iterator;

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
    Camera light_camera = Camera({18.101824f, 13.483343f, -34.785542f}, glm::radians(90.f));

    bool lighting = true;
    bool runtime_lighting = false;
    glm::mat4 light_projview;

    bool depth_rendering = true;
    Mesh depth_mesh;
    glm::mat3 transposed_inversed_model;
    bool render_skybox_status = true;


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
        light_camera.rotate({2.469812, 0.377793, 3.141592}, false);


        material.shininess = 32.f;
        material.ambient = LightColor(0.2);
        material.specular = LightColor(1.f);
        material.diffuse = LightColor(1.f);

        light.type = LightType::Point;
        light.specular = material.specular;
        light.ambient = material.ambient;
        light.diffuse = material.diffuse;
        light.buffer.gen(Monitor::size() * 8.f);


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
            if ((lighting && runtime_lighting) || frame == 0)
            {
                if (frame == 0)
                    std::clog << "Generating light map" << std::endl;
                light.buffer.bind();
                light.buffer.bind_texture();
                light.buffer.view_port({0, 0}, light.buffer.size()).clear_buffer();
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

        command_map[L"skybox"] = cmd
        {
            stream >> p.render_skybox_status;
        };

        command_map[L"depth_rendering"] = cmd
        {
            stream >> p.depth_rendering;
        };

        command_map[L"light_buffer_size"] = cmd
        {
            float x, y;
            stream >> x >> y;
            p.light.buffer.gen(x, y);
        };

        command_map[L"runtime_lighting"] = cmd
        {
            stream >> p.runtime_lighting;
        };

        command_map[L"cam_info"] = cmd
        {
            std::clog << "Pos: " << p.camera->position() << std::endl;
            std::clog << "Rot: " << p.camera->euler_angles() << std::endl;
            std::clog << "Front vector: " << -p.camera->front_vector() << std::endl;
            std::clog << "Right vector: " << -p.camera->right_vector() << std::endl;
            std::clog << "Up vector: " << -p.camera->up_vector() << std::endl;
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
            command_iterator = prev_commands.end();
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
        {
            prev_commands.push_back(current_command);
            command_iterator = prev_commands.end();
            parse_command();
        }

        if (current_key == Key::KEY_UP)
        {
            try
            {
                command_iterator--;
                current_command = *command_iterator;
            }
            catch (...)
            {}
        }

        if (current_key == Key::KEY_DOWN)
        {
            try
            {
                command_iterator++;
                current_command = *command_iterator;
            }
            catch (...)
            {}
        }


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
        light_projview = light_camera.projection(light.buffer.size()) * light_camera.view();
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
        if (!render_skybox_status)
            return;
        ShaderSystem::SkyBox::shader.use().set(ShaderSystem::SkyBox::projview, proj * glm::mat4(glm::mat3(view)));
        skybox.draw();
    }

    void load_resources()
    {
        skybox.load("resources/skybox/skybox.png");
        scene.load_model("resources/scene/scene.gltf", DrawMode::LINEAR, 90);
        scene_matrix = scene.scale(0.05, 0.05, 0.05).model();
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


float rand_float(float min, float max)
{
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (max - min));
}

void box_check(AABB_3D& box)
{
    for (int i = 0; i < 3; i++)
        if (box.min[i] > box.max[i])
            std::swap(box.min[i], box.max[i]);
}

void test_octree(int num = 73)
{
    srand(time(NULL));
    std::clog << "OCTREE TESTING" << std::endl;
    std::list<AABB_3D> boxes;
    for (int i = 0; i < num; i++)
    {
        boxes.push_back({{rand_float(-999999, 999999), rand_float(-999999, 999999), rand_float(-999999, 999999)},
                         {rand_float(-999999, 999999), rand_float(-999999, 999999), rand_float(-999999, 999999)}});
        box_check(boxes.back());
    }

    Engine::Octree<int> tree;
    std::clog << "START TESTING" << std::endl;
    auto begin = std::chrono::steady_clock::now();
    for (auto& box : boxes) tree.push(0, box);
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count();
    std::clog << "Octree generation time: " << time << " milliseconds" << std::endl;
    std::clog << "Added " << num << " objects" << std::endl;
    auto a = tree.aabb();


    std::clog << "Octree aabb: " << a.min << "\t" << a.max << std::endl;
}

int main()
{
    Engine::init();
    Program p;
    //test_octree();
}
