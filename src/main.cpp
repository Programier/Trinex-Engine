#include "Graphics/obj_loader.hpp"
#include <GL/glew.h>
#include <Graphics/camera.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture.hpp>
#include <Graphics/texturearray.hpp>
#include <Window/window.hpp>
#include <glm/ext.hpp>
#include <iostream>


int main()
{
    bool light = false;

    Engine::Window window(1280, 720, "test", true);
    Engine::Shader shader(
            "/home/programier/projects/Engine3D/src/Engine/Graphics/Shaders/main.vert",
            "/home/programier/projects/Engine3D/src/Engine/Graphics/Shaders/main.frag");

    Engine::TextureArray array;
    Engine::Mesh mesh;
    Engine::Mesh basic_mesh(BASIC_TEXTURE);
    try
    {
        Engine::load_obj(array, mesh, "/home/programier/projects/Engine3D/resources/map.obj");
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    Engine::Camera camera(glm::vec3(0, 0, 1), glm::radians(70.f));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 model(1.0f);
    model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
    Engine::print(array.get_max_size(), std::cout) << std::endl;
    shader.use();


    while (window.is_open())
    {
        float speed = 15 * window.event.diff_time();
        window.event.poll_events();
        window.clear_buffer();
        if (window.event.keyboard.just_pressed() == Engine::KEY_TAB)
        {
            window.event.mouse.cursor_status(window.event.mouse.cursor_status() == Engine::NORMAL
                                                     ? Engine::DISABLED
                                                     : Engine::NORMAL);
        }

        if (window.event.keyboard.just_pressed() == Engine::KEY_R)
        {

            shader.load("/home/programier/projects/Engine3D/src/Engine/Graphics/Shaders/main.vert",
                        "/home/programier/projects/Engine3D/src/Engine/Graphics/Shaders/main.frag")
                    .use()
                    .set("max_size", array.get_max_size());
        }

        auto coords = camera.coords();
        shader.set("camera", coords);
        shader.set("light", light);
        shader.set("projview", camera.projection(window) * camera.view());
        std::string title = "x: " + std::to_string(coords.x) + ", y: " + std::to_string(coords.y) +
                            ", z: " + std::to_string(coords.z);
        window.title(title);
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
            std::cout << coords.x << " " << coords.y << " " << coords.z << std::endl;
        }


        if (window.event.keyboard.just_pressed() == Engine::KEY_G)
        {
            light = !light;
        }


        array.bind();
        mesh.draw(Engine::TRIANGLE);

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
