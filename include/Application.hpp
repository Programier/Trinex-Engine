#pragma once
#include <GUI.hpp>
#include <Graphics/camera.hpp>
#include <Graphics/font.hpp>
#include <Graphics/framebuffer.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/texture_2D.hpp>
#include <Window/window.hpp>
#include <list>
#include <Graphics/line.hpp>
#include <Graphics/skybox.hpp>

namespace Engine
{

    class Application
    {
    public:
        Engine::Window window;
        const Engine::Size2D& window_size = window.size();
        bool GUI = false;

        Engine::FrameBuffer framebuffer;
        Size2D viewport_size;
        Engine::Texture2D texture_2D;
        Engine::Texture2D view_image;
        Engine::Texture2D depth_image;
        Engine::Mesh<float> mesh;
        Engine::Line lines;
        Engine::Skybox skybox;
        bool active_mouse = false;

        Size2D monitor_size;

        // Player (Camera)
        Engine::Camera main_camera;
        Engine::Camera* camera = &main_camera;
        glm::mat4 projection;
        glm::mat4 view;
        float speed = 5.f;
        Point3D position = {0.f, 0.f, 0.f};
        EulerAngle3D rotation = {0.f, 0.f, 0.f};
        Size3D model_scale = {0.1, 0.1, 0.1};
        Application& generate_texture(const Size2D& size, bool off = false);


        Application();
        Application& loop();
        Application& change_gui_status();
        Application& load_scene(const std::string& filename);
        Application& load_skybox(const std::string& filename);
        Application& update_model_matrix();
        Application& update_current_camera();
        void camera_proccess();


        ~Application();
    };
}// namespace Engine


int game_main(int argc, char* argv[]);
