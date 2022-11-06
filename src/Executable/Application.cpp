#include <Application.hpp>
#include <Core/logger.hpp>
#include <Graphics/enable_param.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/shader_system.hpp>
#include <iostream>
#include <string>

using namespace Engine;
#define FONT_PATH "resources/fonts/font.otf"

Application::Application()
{
    Engine::init();
    window.init(1280, 720, "Engine", WindowAttrib::WIN_RESIZABLE);

    // Loading font
    font.load(FONT_PATH, {27, 27});

    enable(EnableCap::Blend)(EnableCap::DepthTest);
    blend_func(BlendFunc::SrcAlpha, BlendFunc::OneMinusSrcAlpha);
}

void Application::keyboard_procces()
{
    auto backspace = [this]() {
        if (!text.empty())
            text.pop_back();
    };

    switch (KeyboardEvent::just_pressed())
    {
        case KEY_BACKSPACE:
            backspace();
            break;
        case KEY_ENTER:
            text.push_back(L'\n');
        default:
            break;
    }


    if (KeyboardEvent::get_key_status(KEY_BACKSPACE) == KeyStatus::REPEAT)
    {
        backspace();
    }
}

void Application::render()
{
    wchar_t last = TextEvent::last_symbol();
    if (last)
    {
        text.push_back(last);
    }
    auto projview = glm::ortho(0.f, window.width(), 0.f, window.height(), 0.f, 1.f);

    using namespace ShaderSystem;
    Text::shader.use().set(Text::projview, projview).set(Text::color, Color::Red);
    font.draw(text, {10, window.height() - font.font_size().y});
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
        window.swap_buffers().event.wait_for_event();
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
