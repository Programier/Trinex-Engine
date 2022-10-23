#include <Application.hpp>
#include <Core/engine.hpp>
#include <Core/init.hpp>
#include <Core/logger.hpp>
#include <Core/system.hpp>
#include <Graphics/enable_param.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_system.hpp>
#include <LibLoader/lib_loader.hpp>
#include <Sensors/sensor.hpp>
#include <Window/monitor.hpp>
#include <iostream>


using namespace Engine;
#define FONT_PATH "/home/programier/GameEngine/resources/fonts/font.otf"

Font font;

static void close_app()
{
    Engine::logger->log("Closing Application\n");
    Engine::Window::close();
}

static void pause_app()
{
    Engine::logger->log("Paused application\n");
}


Application& Application::generate_texture(const Size2D& size, bool off)
{
    std::vector<byte> _M_image;
    _M_image.reserve(size.x * size.y * 4);
    for (int i = 0; i < (int) (size.x * size.y); i++)
    {
        for (int j = 0; j < 3; j++) _M_image.push_back(rand() % 256);
        _M_image.push_back(255);
    }

    if (!off)
    {
        TextureParams params;
        params.pixel_type = BufferValueType::UNSIGNED_BYTE;
        params.format = PixelFormat::RGBA;
        texture_2D.create(params).min_filter(TextureFilter::LINEAR).mag_filter(TextureFilter::LINEAR);
        texture_2D.gen(size, 0, (void*) _M_image.data());

        // Check
        std::vector<byte> tmp;
        texture_2D.read_data(tmp);

        if (tmp.size() != _M_image.size())
        {
            logger->log("TEST FAILED\n");
            logger->log("Image size: %d\nGetted size: %d\n", (int) _M_image.size(), tmp.size());
        }
    }
    else
    {
        texture_2D.update({10, 10}, size / 2.f, 0, (void*) _M_image.data());
    }


    texture_2D.base_level(0);
    return *this;
}

Application::Application()
{
    // Window params
    window.set_orientation(WindowOrientation::WIN_ORIENTATION_LANDSCAPE |
                           WindowOrientation::WIN_ORIENTATION_LANDSCAPE_FLIPPED);

    static const char* name = "Engine";

    monitor_size = (system_name == SystemName::ANDROID_OS ? Size2D(Monitor::height(), Monitor::width()) : Monitor::size());

    auto flags = WIN_RESIZABLE | (Engine::system_name == SystemName::ANDROID_OS ? WIN_FULLSCREEN_DESKTOP : 0);
    window.init((Engine::system_name == SystemName::ANDROID_OS ? monitor_size : monitor_size / 2.f), name, flags);

    Engine::Event::on_terminate.push_back(close_app);
    Engine::Event::on_quit.push_back(close_app);
    Engine::Event::on_pause.push_back(pause_app);

    update_current_camera();
    viewport_size = monitor_size;
    Engine::enable(Engine::EnableCap::Blend)(Engine::EnableCap::DepthTest);
    Engine::blend_func(Engine::BlendFunc::SrcAlpha, Engine::BlendFunc::OneMinusSrcAlpha);


    srand(time(nullptr));
    generate_texture({100, 100});
    // font.load("/home/programier/SHARED_FOLDER/Bou_Collegiate.ttf", {0, 48});
    //font.font_path("/home/programier/GameEngine/resources/fonts/font.otf");

    MeshInfo& info = mesh;
    info.mode = DrawMode::STATIC_DRAW;
    mesh.data = {-1, -1, 0, 0, 0, 1, 1, 0, 1, 1, -1, 1, 0, 0, 1, -1, -1, 0, 0, 0, 1, 1, 0, 1, 1, 1, -1, 0, 1, 0};
    mesh.vertices = 6;
    mesh.attributes = {{3, BufferValueType::FLOAT}, {2, BufferValueType::FLOAT}};
    mesh.gen().set_data(mesh.data.data()).update_atributes();
    lines.scale(model_scale);

    //skybox.load("/home/programier/GameEngine/resources/skybox/skybox.png");
    font.load(FONT_PATH, {48, 48});
}


Application& Application::update_current_camera()
{
    (*camera).move(position, false);
    camera->rotate(rotation, false);

    return *this;
}


void Application::camera_proccess()
{
    auto diff = static_cast<float>(Event::diff_time()) / (1000.f);

    if (KeyboardEvent::just_pressed() == KEY_G)
    {
        generate_texture({100, 100}, true);
    }

    if (KeyboardEvent::just_pressed() == KEY_Y)
    {
        if (texture_2D.base_level() < texture_2D.max_lod_level())
            texture_2D.base_level(texture_2D.base_level() + 1);
    }

    if (KeyboardEvent::just_pressed() == KEY_U)
    {
        if (texture_2D.base_level())
            texture_2D.base_level(texture_2D.base_level() - 1);
    }

    if (KeyboardEvent::just_pressed() == KEY_SPACE)
    {
        active_mouse = !active_mouse;

        if (active_mouse)
        {
            window.cursor_mode(CursorMode::HIDDEN);
        }
        else
        {
            window.cursor_mode(CursorMode::NORMAL);
        }
    }

    if (KeyboardEvent::just_pressed() == KEY_M)
    {
        texture_2D.generate_mipmap();
    }


    if (KeyboardEvent::pressed(Engine::KEY_W))
        camera->move(-camera->front_vector() * speed * diff);

    if (KeyboardEvent::pressed(Engine::KEY_S))
        camera->move(camera->front_vector() * speed * diff);

    if (KeyboardEvent::pressed(Engine::KEY_A))
        camera->move(-camera->right_vector() * speed * diff);

    if (KeyboardEvent::pressed(Engine::KEY_D))
        camera->move(camera->right_vector() * speed * diff);

    if (KeyboardEvent::pressed(Engine::KEY_UP))
        camera->move(Engine::Constants::OY * speed * diff);

    if (KeyboardEvent::pressed(Engine::KEY_DOWN))
        camera->move(-Engine::Constants::OY * speed * diff);

    if (!active_mouse || (Engine::system_name == SystemName::ANDROID_OS && TouchScreenEvent::fingers_count() == 0))
        return;

    MouseEvent::position(window_size / 2.f);
    Offset2D mouse_offset = Engine::system_name == SystemName::ANDROID_OS
                                    ? TouchScreenEvent::get_finger(0).offset * window_size
                                    : Engine::MouseEvent::offset();
    mouse_offset = 2.f * mouse_offset / (this->window_size);

    camera->rotate(-mouse_offset.x, Engine::Constants::OY);
    camera->rotate(-mouse_offset.y, camera->right_vector());
}

Application& Application::loop()
{
    namespace sh_scene = Engine::ShaderSystem::Line;
    namespace sh_skybox = Engine::ShaderSystem::SkyBox;
    namespace sh_text = Engine::ShaderSystem::Text;

    while (window.is_open())
    {
        camera_proccess();

        if (window.event.keyboard.just_pressed() == KEY_TAB ||
            (window.event.touchscreen.fingers_count() == 3 && window.event.touchscreen.prev_fingers_count() != 3))
        {
            change_gui_status();
        }

        auto size = (GUI ? viewport_size : window_size);
        framebuffer.bind().clear_buffer(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT).view_port({0, 0}, size);


        projection = camera->projection(size);
        view = camera->view();

        sh_scene::shader.use()
                .set(sh_scene::projview, projection * view)
                .set(sh_scene::model, lines.model())
                .set(sh_scene::color, Color::Red);

        lines.draw();


        sh_skybox::shader.use().set(sh_skybox::projview, projection * glm::mat4(glm::mat3(view)));
        skybox.draw();

        if (GUI)
        {
            window.bind().update_view_port().clear_buffer(BufferBitType::COLOR_BUFFER_BIT | BufferBitType::DEPTH_BUFFER_BIT);
            GUI::render();
        }

        window.bind().update_view_port();
        sh_text::shader.use()
                .set(sh_text::color, Color::Red)
                .set(sh_text::projview, glm::ortho(0.f, window_size.x, 0.f, window_size.y, 0.f, 1.f));
        font.draw(L"Hello", {200, 200}, 0.5);

        window.swap_buffers();
        window.event.poll_events();
    }

    window.close();
    return *this;
}

Application& Application::change_gui_status()
{
    if (GUI == false)
    {
        GUI::init(this);
        framebuffer.gen(FrameBufferType::DRAW_FRAMEBUFFER);
        TextureParams params;
        params.border = false;
        params.format = PixelFormat::RGBA;
        params.pixel_type = BufferValueType::UNSIGNED_BYTE;
        params.type = TextureType::Texture_2D;
        view_image.create(params);
        view_image.gen(monitor_size);

        params.format = PixelFormat::DEPTH;
        params.pixel_type = BufferValueType::UNSIGNED_SHORT;

        depth_image.create(params);
        depth_image.gen(monitor_size);


        framebuffer.attach_texture(view_image, FrameBufferAttach::COLOR_ATTACHMENT, 0)
                .attach_texture(depth_image, FrameBufferAttach::DEPTH_ATTACHMENT);
    }
    else
    {
        GUI::terminate();
        framebuffer = FrameBuffer();
        viewport_size = window_size;
    }

    GUI = !GUI;

    return *this;
}


Application::~Application()
{
    if (GUI)
        GUI::terminate();
    window.close();
    //  delete font;
}


Application* parse_args(int argc, char** argv, Application* app)
{
#ifdef __ANDROID__
    if (argc > 1)
        Engine::library_dir = argv[1];
#else
    Engine::library_dir = "/home/programier/Projects/bin";
#endif

    //app->change_gui_status();
    return app;
}

Application& Application::update_model_matrix()
{
    lines.scale(this->model_scale, false);
    return *this;
}

Application& Application::load_scene(const std::string& filename)
{
    std::clog << "Loading scene: " << filename << std::endl;
    //    Engine::Image img(filename, true);
    //    TextureParams params;
    //    //    params.copy_data = true;
    //    //    params.data = (void*) img.vector().data();
    //    params.pixel_type = BufferValueType::UNSIGNED_BYTE;
    //    params.format = img.channels() == 4 ? PixelFormat::RGBA : PixelFormat::RGB;
    //    //    params.size = img.size();
    //    texture_2D.create(params);

    lines.load_from(filename);
    return *this;
}

Application& Application::load_skybox(const std::string& filename)
{
    skybox.load(filename);
    return *this;
}

int game_main(int argc, char* argv[])
try
{
    init();
    auto app = new Application();
    parse_args(argc, argv, app)->loop();
    delete app;
    return 0;
}
catch (const std::exception& e)
{
    std::clog << e.what() << std::endl;
    return 1;
}
