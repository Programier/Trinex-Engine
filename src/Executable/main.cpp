#include <BasicFunctional/basic_functional.hpp>
#include <GL/glew.h>
#include <Graphics/camera.hpp>
#include <Graphics/heightmap.hpp>
#include <Graphics/hitbox.hpp>
#include <Graphics/line.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/skybox.hpp>
#include <Graphics/terrainmodel.hpp>
#include <Graphics/text.hpp>
#include <Graphics/texture.hpp>
#include <Graphics/texturearray.hpp>
#include <Physics/terrain_collision.hpp>
#include <Window/window.hpp>
#include <iostream>
#include <thread>


int main()
{

    bool light = false;
    Engine::Window window(1280, 720, "test", true);
    Engine::Shader skybox_shader("Shaders/skybox.vert", "Shaders/skybox.frag");

    Engine::Shader shader("Shaders/main.vert", "Shaders/main.frag");
    Engine::TerrainModel model("resources/de_dust2/de_dust2.obj", Engine::NEAREST, 4);

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
    Engine::ObjectParameters player = {camera.position(), {0.f, 0.f, 0.f}, 4, 1};

    while (window.is_open())
    {
        player.force = glm::vec3(0, player.force[1], 0);
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

        player.position = camera.position();
        const auto& offset = window.event.mouse.offset();
        if (window.event.mouse.cursor_status() == Engine::DISABLED)
        {
            const auto& up = camera.up_vector();
            float angle = Engine::angle_between(up, Engine::OY) * (camera.front_vector()[1] < 0 ? -1 : 1);
            float y_offset = offset.y * 2 / (window.height());
            float x_offset = offset.x * 2 / (window.width());
            camera.rotate(x_offset, Engine::OY);
            bool rotate = glm::abs(angle) <= glm::radians(89.f) || angle * y_offset > 0;
            if (rotate)
                camera.rotate(y_offset, camera.right_vector());
        }

        if (window.event.pressed(Engine::KEY_W))
        {
            player.force.z = speed;
        }

        if (window.event.pressed(Engine::KEY_S))
        {
            player.force.z = -speed;
        }

        if (window.event.pressed(Engine::KEY_A))
        {
            player.force.x = -speed;
        }

        if (window.event.pressed(Engine::KEY_D))
        {
            player.force.x = speed;
        }

        if (window.event.keyboard.just_pressed() == Engine::KEY_SPACE)
        {
            player.force.y = 0.25;
        }

        if (window.event.pressed(Engine::KEY_UP))
        {
            player.position.y += speed;
        }

        if (window.event.pressed(Engine::KEY_DOWN))
        {
            player.position.y -= speed;
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

        camera.move(player.position, false);

        camera.move(player.force, Engine::remove_coord(camera.right_vector(), Engine::Coord::Y), Engine::OY,
                    Engine::remove_coord(camera.front_vector(), Engine::Coord::Y));

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
                mode = Engine::FULLSCREEN;
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
