#include <Core/logger.hpp>
#include <Core/string_format.hpp>
#include <Graphics/animator.hpp>
#include <Graphics/enable_param.hpp>
#include <Graphics/ray.hpp>
#include <Graphics/shader_system.hpp>
#include <Graphics/textured_object.hpp>
#include <ImGui/imgui.h>
#include <TemplateFunctional/instanceof.hpp>
#include <Window/monitor.hpp>
#include <editor.hpp>
#include <iostream>
#include <render_texture.hpp>
#include <resouces.hpp>
#include <texture_view.hpp>
#include <viewport.hpp>

namespace Editor
{
    void ViewPort::move_camera()
    {}

    ViewPort::ViewPort()
    {}

    void ViewPort::render()
    {}

    void ViewPort::proccess_commands()
    {}
}// namespace Editor
