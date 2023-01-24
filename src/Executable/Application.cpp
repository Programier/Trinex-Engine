#include <Application.hpp>
#include <Core/logger.hpp>
#include <Core/string_convert.hpp>
#include <Core/string_format.hpp>
#include <Graphics/enable_param.hpp>
#include <Graphics/hitbox.hpp>
#include <Graphics/line.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/resources.hpp>
#include <Graphics/shader_system.hpp>
#include <Graphics/textured_object.hpp>
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
std::string SCENE_PATH = "/home/programier/Games/new/scene.gltf";
#endif
#include <iostream>

std::list<Animator*> animators;
int test();

AnimatedTexturedObject* cube;
Application::Application()
{

    window.set_orientation(WindowOrientation::WIN_ORIENTATION_LANDSCAPE);
    window.init(1280, 720, "Engine", WindowAttrib::WIN_RESIZABLE);
    StaticTexturedObject::load(SCENE_PATH, &scene);
    for (auto& drawable : scene.drawables())
    {
        auto a = dynamic_cast<AnimatedTexturedObject*>(drawable);
        if (a)
        {
            for (auto& anim : a->animations())
            {
                animators.push_back(Object::new_instance<Animator>());
                animators.back()->animation(anim);
            }
        }
    }

    enable(EnableCap::Blend)(EnableCap::DepthTest);
    blend_func(BlendFunc::SrcAlpha, BlendFunc::OneMinusSrcAlpha);
    if (!scene.active_camera())
        scene.active_camera(Object::new_instance<Camera>(Constants::zero_vector, glm::radians(50.f)));
    camera = scene.set_as_active_scene().active_camera();
    // camera->move(320.424988, 2.605027, -82.476044, false);
    camera->max_render_distance(10000);
}

#define on_pressed(key, do_action)                                                                                     \
    if (KeyboardEvent::pressed(key))                                                                                   \
    {                                                                                                                  \
        do_action                                                                                                      \
    }


#define on_just_pressed(key, do_action)                                                                                \
    if (KeyboardEvent::just_pressed() == key)                                                                          \
    {                                                                                                                  \
        do_action                                                                                                      \
    }

void Application::keyboard_procces()
{
    static const float _M_speed = 100.f;
    float speed = _M_speed * Event::diff_time();

    on_pressed(KEY_W, { camera->move(camera->front_vector() * speed); });
    on_pressed(KEY_S, { camera->move(camera->front_vector() * -speed); });
    on_pressed(KEY_A, { camera->move(camera->right_vector() * -speed); });
    on_pressed(KEY_D, { camera->move(camera->right_vector() * speed); });
    on_just_pressed(KEY_V, { window.vsync(!window.vsync()); });

    on_pressed(KEY_SPACE,
               { camera->move(Constants::OY * speed * (KeyboardEvent::pressed(KEY_LEFT_SHIFT) ? -1.f : 1.f)); });


    static bool use_mouse = false;
    if (MouseEvent::just_pressed() == MOUSE_BUTTON_MIDDLE)
    {
        use_mouse = !use_mouse;
        MouseEvent::relative_mode(use_mouse);
    }

    if (use_mouse)
    {
        auto offset = -2.f * MouseEvent::offset() / window.size();

        camera->rotate(offset.x, Constants::OY);
        camera->rotate(offset.y, camera->right_vector());
    }
}


void Application::render()
{
    //for (auto& a : animators) a.tick(Event::diff_time());

    camera->aspect(window.width() / window.height());
    namespace sh = ShaderSystem::Scene;
    sh::shader.use().set(sh::projview, camera->projection() * camera->view()).set(sh::lighting, 0);

    scene.render();
    //for (auto& drawable : scene.drawables()) drawable->aabb().render(drawable->scene_nodes().begin()->second->global_matrix());

    if (Event::frame_number() % 60 == 0)
    {
        logger->log("FPS: %1.f\n", 1.f / Event::diff_time());
        //        for (auto& drawable : scene.drawables())
        //            logger->log("Drawable pos: %s",
        //                        Strings::format("{}", drawable->aabb().apply_model(drawable->scene_nodes().begin()->second->global_matrix()).center()).c_str());
    }
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
    for (int i = 0; i < argc; i++)
    {
        if (std::string(argv[i]) == "--test")
        {
            test();
            goto exit;
        }
    }

    Engine::init(EngineAPI::OpenGL);
    printf("Arguments count: %d\n", argc);
    if (argc > 1)
        SCENE_PATH = argv[1];
    delete &((new Application())->loop());

exit:
    terminate();
    return 0;
}
catch (const std::exception& e)
{
    Engine::logger->log("%s\n", e.what());
    return 1;
}
