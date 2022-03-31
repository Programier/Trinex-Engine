#include <GL/glew.h>
#include <Graphics/camera.hpp>
#include <Graphics/heightmap.hpp>
#include <Graphics/line.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/model.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/skybox.hpp>
#include <Graphics/text.hpp>
#include <Graphics/texture.hpp>
#include <Graphics/texturearray.hpp>
#include <Phisics/terrain_collision.hpp>
#include <Window/window.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include <thread>

int main()
{
    bool light = false;

    Engine::Window window(1280, 720, "test", true);
    Engine::Shader skybox_shader("Shaders/skybox.vert", "Shaders/skybox.frag");

    Engine::Shader shader("Shaders/main.vert", "Shaders/main.frag");
    Engine::Model model("resources/de_dust2/de_dust2.obj", Engine::NEAREST, 80);

    Engine::Skybox skybox(std::vector<std::string>{"resources/skybox/right.jpg", "resources/skybox/left.jpg",
                                                   "resources/skybox/top.jpg", "resources/skybox/bottom.jpg",
                                                   "resources/skybox/front.jpg", "resources/skybox/back.jpg"});

    Engine::Shader line("Shaders/lines.vert", "Shaders/lines.frag");

    Engine::Camera camera(glm::vec3(65.6901f, 10.5f, -146.858f), glm::radians(70.f));
    camera.rotate(-0.130556, -3.32162, 0);
    Engine::Line lines;
    lines.lines_from(model).line_width(0.5f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Text renderer
    Engine::Shader text_shader("Shaders/text.vert", "Shaders/text.frag");
    Engine::Text text_renderer("resources/fonts/STIX2Text-Bold.otf", 25);
    glm::mat4 model_matrix =
            glm::scale(glm::rotate(glm::mat4(1.0f), glm::radians(90.f), {-1., 0., 0.f}), {0.05f, 0.05f, 0.05f});
    Engine::HeightMap height_map(model, 1.f, model_matrix);

    bool lines_draw = false;
    std::string LOG_POS;
    std::string FPS;

    unsigned int frame = 0;
    Engine::ObjectParameters player = {camera.coords(), {0.f, 0.f, 0.f}, 4};


    while (window.is_open())
    {
        player.force = glm::vec3(0, 0, player.force[2]);
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
            line.load("Shaders/lines.vert", "Shaders/lines.frag");
        }

        player.position = camera.coords();
        const auto& offset = window.event.mouse.offset();
        if (window.event.mouse.cursor_status() == Engine::DISABLED)
            camera.rotate(offset.y * 2 / (window.height()), -offset.x * 2 / (window.width()), 0);

        if (window.event.pressed(Engine::KEY_W))
        {
            player.force[0] = speed;
        }

        if (window.event.pressed(Engine::KEY_S))
        {
            player.force[0] = -speed;
        }

        if (window.event.pressed(Engine::KEY_A))
        {
            player.force[1] = -speed;
        }

        if (window.event.pressed(Engine::KEY_D))
        {
            player.force[1] = speed;
        }

        if (window.event.keyboard.just_pressed() == Engine::KEY_SPACE)
        {
            player.force[2] = 0.25;
        }

        if (window.event.keyboard.just_pressed() == Engine::KEY_C)
        {
            auto coords = camera.coords();
            std::cout << "Coords: " << coords.x << " " << coords.y << " " << coords.z << std::endl;
            auto rotation = camera.rotation();
            std::cout << "Rotation: " << rotation.x << " " << rotation.y << " " << rotation.z << std::endl;
            const auto& l = model.limits();

            auto print = [](std::string name, auto a) {
                std::cout << name << ": {" << a.x << " : " << a.y << " : " << a.z << "}" << std::endl;
            };
            print("Min: ", l.min);
            print("Max: ", l.max);
        }

        if (window.event.pressed(Engine::KEY_LEFT_SHIFT))
        {
            if (player.height > 2)
            {
                player.height -= 0.2;
                player.position.y -= 0.2;
            }
        }
        else
        {

            if (player.height < 4)
            {
                player.height += 0.2;
                player.position.y += 0.2;
            }
        }


        if (window.event.keyboard.just_pressed() == Engine::KEY_L)
        {
            light = !light;
        }

        if (window.event.keyboard.just_pressed() == Engine::KEY_Y)
            lines_draw = !lines_draw;

        player = Engine::check_terrain_collision(height_map, {player})[0];
        camera.coords(player.position);
        auto rotation = camera.rotation();

        camera.move_along_axes(player.force[0], player.force[1], player.force[2],
                               {-glm::sin(rotation.y), 0, -glm::cos(rotation.y)},
                               {glm::cos(rotation.y), 0, -glm::sin(rotation.y)}, {0.f, 1.f, 0.f});
        auto projection = camera.projection(window);
        auto projview = projection * camera.view();

        if (lines_draw == false)
        {
            shader.use()
                    .set("camera", player.position)
                    .set("light", light)
                    .set("projview", projview)
                    .set("model", model_matrix);
            model.draw();
        }
        else
        {
            line.use()
                    .set("projview", projview)
                    .set("color", glm::vec3(1, 0, 0))
                    .set("model", model_matrix)
                    .set("light", light)
                    .set("camera", player.position);
            lines.draw();
        }
        skybox_shader.use()
                .set("projview", camera.projection(window) * glm::mat4(glm::mat3(camera.view())))
                .set("light", light);
        skybox.draw();

        if (window.event.keyboard.just_pressed() == Engine::KEY_ENTER)
        {
            auto mode = window.mode();
            if (mode == Engine::NONE)
            {
                mode = Engine::FULLSCREEN;
            }
            else
                mode = Engine::NONE;
            window.mode(mode);
        }

        LOG_POS = "X: " + std::to_string(player.position.x) + ", Y: " + std::to_string(player.position.y) +
                  ", Z: " + std::to_string(player.position.z);
        auto w_size = window.size();
        text_shader.use().set("color", glm::vec3(1, 1, 1)).set("projview", glm::ortho(0.0f, w_size.x, 0.0f, w_size.y));
        text_renderer.draw(LOG_POS, 5, w_size.y - 25, 1);
        if (frame++ % 30 == 0)
            FPS = "FPS: " + std::to_string(int(1 / window.event.diff_time()));
        text_renderer.draw(FPS, 5, w_size.y - 55, 1);

        window.swap_buffers();
    }

    return 0;
}
