#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/colors.hpp>
#include <Engine/default_client.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/rhi.hpp>
#include <Window/window.hpp>
#include <Graphics/shader_parameters.hpp>
#include <Graphics/pipeline.hpp>
#include <Core/logger.hpp>

namespace Engine
{
    DefaultClient::DefaultClient()
    {
        m_vertex_buffer         = Object::new_instance<TexCoordVertexBuffer>();
        m_vertex_buffer->buffer = {{-1.0, -1.0}, {-1.f, 1.f}, {1.f, 1.f}, {-1.0, -1.0}, {1.f, 1.f}, {1.f, -1.f}};
        m_vertex_buffer->init_resource();

        m_material = Object::load_object("Test::Triangle")->instance_cast<Material>();
        info_log("DefaultClient", "Binding index of globals is '%d'", int(m_material->pipeline->global_parameters.bind_index()));
    }

    DefaultClient& DefaultClient::on_bind_viewport(class RenderViewport* viewport)
    {
     //   viewport->window()->imgui_initialize();
        return *this;
    }

    DefaultClient& DefaultClient::render(class RenderViewport* viewport)
    {
        viewport->rhi_bind();
        float x = (glm::sin(engine_instance->time_seconds()) + 1.f) / 2.f;
        float y = (glm::cos(engine_instance->time_seconds()) + 1.f) / 2.f;
        viewport->rhi_clear_color(Color(x, y, 0.f, 1.f));


        static Name factor_name = "factor";
        auto factor_param = m_material->find_parameter(factor_name);


        GlobalShaderParameters params {};

        params.viewport = color;
        rhi->push_global_params(params);

        if(factor_param)
        {
            reinterpret_cast<FloatMaterialParameter*>(factor_param)->param = factor;
        }

        m_material->apply();
        m_vertex_buffer->rhi_bind(0);
        rhi->draw(6, 0);

        rhi->pop_global_params();

        //viewport->window()->imgui_window()->render();
        return *this;
    }

    DefaultClient& DefaultClient::update(class RenderViewport* viewport, float dt)
    {
        // auto* window = viewport->window()->imgui_window();
        // window->new_frame();
        // ImGui::Begin("Hello World");
        // ImGui::ColorEdit4("Color", &color.x);
        // ImGui::SliderFloat("Factor", &factor, 0.f, 1.f);
        // ImGui::End();
        // window->end_frame();

        return *this;
    }

    implement_engine_class_default_init(DefaultClient, 0);
}// namespace Engine
