#include <GL/glew.h>
#include <Graphics/camera.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/model.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/skybox.hpp>
#include <Graphics/texture.hpp>
#include <Graphics/texturearray.hpp>
#include <Window/window.hpp>
#include <glm/ext.hpp>
#include <iostream>

int main()
{
    bool light = false;

    Engine::Window window(1280, 720, "test", true);
    Engine::Shader skybox_shader("Shaders/skybox.vert", "Shaders/skybox.frag");

    Engine::Shader shader("Shaders/main.vert", "Shaders/main.frag");
    Engine::Model model("resources/Downtown_Damage_0.obj");

    Engine::Skybox skybox(std::vector<std::string>{"resources/skybox/right.jpg", "resources/skybox/left.jpg",
                                                   "resources/skybox/top.jpg", "resources/skybox/bottom.jpg",
                                                   "resources/skybox/front.jpg", "resources/skybox/back.jpg"});

    Engine::Camera camera(glm::vec3(-7.35696, 25.5047, -92.4169), glm::radians(70.f));
    camera.rotate(-0.0777782, -0.139584, 0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    while (window.is_open())
    {
        float speed = 15 * window.event.diff_time();
        window.event.poll_events();
        window.clear_buffer();
        if (window.event.keyboard.just_pressed() == Engine::KEY_TAB)
        {
            window.event.mouse.cursor_status(window.event.mouse.cursor_status() == Engine::NORMAL ? Engine::DISABLED
                                                                                                  : Engine::NORMAL);
        }

        if (window.event.keyboard.just_pressed() == Engine::KEY_R)
        {
            shader.load("Shaders/main.vert", "Shaders/main.frag");
            skybox_shader.load("Shaders/skybox.vert", "Shaders/skybox.frag");
        }

        auto coords = camera.coords();
        const auto& offset = window.event.mouse.offset();
        if (window.event.mouse.cursor_status() == Engine::DISABLED)
            camera.rotate(offset.y / (window.height()), -offset.x / (window.width()), 0);

        if (window.event.pressed(Engine::KEY_W))
        {
            camera.move(speed, 0, 0);
        }

        if (window.event.pressed(Engine::KEY_S))
        {
            camera.move(-speed, 0, 0);
        }

        if (window.event.pressed(Engine::KEY_A))
        {
            camera.move(0, -speed, 0);
        }

        if (window.event.pressed(Engine::KEY_D))
        {
            camera.move(0.0, speed, 0);
        }

        if (window.event.keyboard.just_pressed() == Engine::KEY_C)
        {
            auto coords = camera.coords();
            std::cout << "Coords: " << coords.x << " " << coords.y << " " << coords.z << std::endl;
            auto rotation = camera.rotation();
            std::cout << "Rotation: " << rotation.x << " " << rotation.y << " " << rotation.z << std::endl;
        }


        if (window.event.keyboard.just_pressed() == Engine::KEY_L)
        {
            light = !light;
        }

        auto projection = camera.projection(window);
        shader.use().set("camera", coords).set("light", light).set("projview", projection * camera.view());
        model.draw();
        skybox_shader.use()
                .set("projview", camera.projection(window) * glm::mat4(glm::mat3(camera.view())))
                .set("light", light);
        skybox.draw();

        if (window.event.keyboard.just_pressed() == Engine::KEY_ENTER)
        {
            auto mode = window.mode();
            if (mode == Engine::NONE)
                mode = Engine::FULLSCREEN;
            else
                mode = Engine::NONE;
            window.mode(mode);
        }
        window.swap_buffers();
    }

    return 0;
}
