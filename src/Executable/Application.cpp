#include <Application.hpp>
#include <Core/logger.hpp>
#include <Core/string_convert.hpp>
#include <Core/string_format.hpp>
#include <Graphics/enable_param.hpp>
#include <Graphics/hitbox.hpp>
#include <Graphics/line.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/shader_system.hpp>
#include <TemplateFunctional/instanceof.hpp>
#include <iostream>
#include <string>

#include <Graphics/shader_system.hpp>
using namespace Engine;

#ifdef __ANDROID__
#define FONT_PATH "/sdcard/GameEngine/resources/fonts/font.otf"
#define SCENE_PATH "/sdcard/GameEngine/resources/scene/new/scene.gltf"
#else
#define FONT_PATH "resources/fonts/font.otf"
#define SCENE_PATH "resources/scene/simple_scene.glb"
#endif

Application::Application()
{
    Engine::init();
    window.set_orientation(WindowOrientation::WIN_ORIENTATION_LANDSCAPE);
    window.init(1280, 720, "Engine", WindowAttrib::WIN_RESIZABLE);
    logger->log("Load font: '%s'\n", FONT_PATH);
    font.load(FONT_PATH, {26, 26});

    int i = 0;

    //scene.load(SCENE_PATH, ObjectLoader::TexturedObjectLoader());

    for (DrawableObject& ell : scene)
    {
        if (is_instance_of<Line>(ell))
        {
            i = i == 0 ? 1 : i << 2;
            if (i == 4)
            {
                dynamic_cast<Line*>(&ell)->color = Color::Red;
            }
        }
    }


    Camera* camera = new Camera();
    camera->move(-camera->front_vector() * 4.f);
    camera->name = L"MainCamera";
    scene.name(L"Scene");
    scene.set_as_active_scene().add_camera(camera).active_camera(camera);
    scene.load(SCENE_PATH, ObjectLoader::TexturedObjectLoader());
    //scene.visible(false);

    // scene.scale({100.f, 100.f, 100.f});

    enable(EnableCap::Blend)(EnableCap::DepthTest);
    blend_func(BlendFunc::SrcAlpha, BlendFunc::OneMinusSrcAlpha);
    camera->max_render_distance(100.f).viewing_angle(glm::radians(70.f)).min_render_distance(1.f);
    ShaderSystem::Line::shader.use().set(ShaderSystem::Line::color, Color::Red);
    //enable(EnableCap::CullFace);
}

#ifdef __ANDROID__
static const Finger* get_finger(bool left)
{
    if (left)
    {
        unsigned int fingers = TouchScreenEvent::fingers_count();

        for (unsigned int i = 0; i < fingers; i++)
        {
            if (TouchScreenEvent::get_finger(i).position.x < Window::width() / 2.f)
                return &TouchScreenEvent::get_finger(i);
        }
        return nullptr;
    }

    unsigned int fingers = TouchScreenEvent::fingers_count();

    for (unsigned int i = 0; i < fingers; i++)
    {
        if (TouchScreenEvent::get_finger(i).position.x > Window::width() / 2.f)
            return &TouchScreenEvent::get_finger(i);
    }
    return nullptr;
}

#endif

static Offset2D get_offset()
{
#ifdef __ANDROID__
    if (TouchScreenEvent::fingers_count())
        return get_finger(true)->offset;
    return {0, 0};

#else

    return (1.5f * MouseEvent::offset() / (Window::size()));
#endif
}

void Application::keyboard_procces()
{

    bool update_speed = false;

#define A 0.95f
#define B 0.05f


    auto& active_camera = scene.active_camera();
    auto& camera = *active_camera.camera;

    static Vector3D move_vector = Constants::zero_vector;

    // Calculate move vector

    if (KeyboardEvent::pressed(KEY_W))
    {
        move_vector = glm::normalize(move_vector * A + camera.front_vector() * B);
        update_speed = true;
    }

    if (KeyboardEvent::pressed(KEY_S))
    {
        move_vector = glm::normalize(move_vector * A - camera.front_vector() * B);
        update_speed = true;
    }

    if (KeyboardEvent::pressed(KEY_A))
    {
        move_vector = glm::normalize(move_vector * A - camera.right_vector() * B);
        update_speed = true;
    }

    if (KeyboardEvent::pressed(KEY_D))
    {
        move_vector = glm::normalize(move_vector * A + camera.right_vector() * B);
        update_speed = true;
    }

    if (KeyboardEvent::pressed(KEY_SPACE))
    {
        move_vector = glm::normalize(move_vector * A + Constants::OY * B);
        update_speed = true;
    }

    if (KeyboardEvent::pressed(KEY_LEFT_SHIFT))
    {
        move_vector = glm::normalize(move_vector * A - Constants::OY * B);
        update_speed = true;
    }

    if (KeyboardEvent::just_pressed() == KEY_U)
    {
        camera.model(Constants::identity_matrix);
    }

    if (KeyboardEvent::just_pressed() == KEY_F)
    {
        window.attribute(WIN_FULLSCREEN_DESKTOP, !window.attribute(WIN_FULLSCREEN_DESKTOP));
    }


    if (KeyboardEvent::just_pressed() == KEY_V)
    {
        window.vsync(!window.vsync());
    }

    float current_speed = 10 * Event::diff_time();
    static float prev_speed = 0.f;

    float speed = prev_speed * A + (update_speed ? current_speed : -current_speed) * B;
    if (speed < 0)
    {
        move_vector = Constants::zero_vector;
        speed = 0.f;
    }

    prev_speed = speed;

    camera.move(move_vector * speed);


    static bool use_mouse = false;

    if (KeyboardEvent::just_pressed() == KEY_UP || TouchScreenEvent::fingers_count() == 3)
    {
        use_mouse = !use_mouse;
        MouseEvent::relative_mode(use_mouse);
    }

    if (use_mouse)
    {
        static Offset2D prev_offset = Constants::zero_vector;
        auto offset = prev_offset * A + get_offset() * B;
        prev_offset = offset;

        camera.rotate(-offset.x, Constants::OY);
        camera.rotate(-offset.y, camera.right_vector());


        scene.scale(Constants::identity_vector + MouseEvent::scroll_offset().y * 0.1f);
    }

    active_camera.update_info();
}

void Application::render()
{

    scene.render();


    int FPS = static_cast<int>(1.f / Event::diff_time());
    static std::string text;

    namespace sh_t = ShaderSystem::Text;

    if (Event::frame_number() % 30 == 0)
    {
        text = Strings::format("FPS: {}\nPos: {}\n", FPS, scene.active_camera().camera->position());
    }

    ShaderSystem::Text::shader.use()
            .set("color", Color::Red)
            .set("projview", glm::ortho(0.f, window.width(), 0.f, window.height(), 0.f, 1.f));
    font.draw(text, {10, window.size().y - 26});
}


Application& Application::loop()
{
    //window.start_text_input();
    TextEvent::enable_text_writing = true;

    while (window.is_open())
    {
        keyboard_procces();
        window.clear_buffer();

        render();
        window.swap_buffers().event.poll_events();
    }
    return *this;
}


Application::~Application()
{}

int game_main(int argc, char* argv[])
try
{
    delete &((new Application())->loop());
    return 0;
}
catch (const std::exception& e)
{
    Engine::logger->log("%s\n", e.what());
    return 1;
}
