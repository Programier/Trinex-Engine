#include <Core/api_object.hpp>
#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/engine.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/renderer.hpp>
#include <ImGui/imgui.h>
#include <Window/window.hpp>


namespace Engine
{

    class ImGuiTest : public Engine::CommandLet
    {
    public:
        int execute(int argc, char** argv)
        {
            Window window({1280, 720}, "ImGUI Test", WindowAttrib::WinResizable);

            ImGuiRenderer::init();

            while (window.is_open())
            {
                engine_instance->renderer()->begin();
                window.bind();

                // Start the ImGui frame
                ImGuiRenderer::new_frame();

                // Draw the ImGui window
                ImGui::Begin("Hello, world!");
                ImGui::Text("This is some text.");
                ImGui::End();

                ImGuiRenderer::render();

                engine_instance->renderer()->end();


                window.swap_buffers();

                Event::poll_events();
            }
            return 0;
        }
    };


    register_class(ImGuiTest, Engine::CommandLet);
}// namespace Engine
