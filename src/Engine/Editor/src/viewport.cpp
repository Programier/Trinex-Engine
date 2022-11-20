#include <Core/logger.hpp>
#include <Graphics/line.hpp>
#include <Graphics/ray.hpp>
#include <ImGui/imgui.h>
#include <Window/monitor.hpp>
#include <iostream>
#include <resouces.hpp>
#include <viewport.hpp>

namespace Editor
{
    static Engine::Size2D to_engine_size(const ImVec2& size)
    {
        return Engine::Size2D(size.x, size.y);
    }

    void ViewPort::move_camera()
    {
        Engine::Camera* camera = Resources::scene.active_camera().camera;

        static const float speed = 15.f;
        const float delta_time = Engine::Event::diff_time();

        if (Engine::KeyboardEvent::pressed(Engine::KEY_W))
            camera->move(camera->front_vector() * speed * delta_time);

        if (Engine::KeyboardEvent::pressed(Engine::KEY_S))
            camera->move(-camera->front_vector() * speed * delta_time);

        if (Engine::KeyboardEvent::pressed(Engine::KEY_A))
            camera->move(-camera->right_vector() * speed * delta_time);

        if (Engine::KeyboardEvent::pressed(Engine::KEY_D))
            camera->move(camera->right_vector() * speed * delta_time);

        if (Engine::KeyboardEvent::pressed(Engine::KEY_SPACE))
            camera->move(camera->up_vector() * speed * delta_time *
                         (Engine::KeyboardEvent::pressed(Engine::KEY_LEFT_SHIFT) ? -1.f : 1.f));

        if (Engine::KeyboardEvent::just_pressed() == Engine::KEY_ENTER)
            camera->move(Resources::object_for_rendering->position(), false);

        static const auto& offset = Engine::MouseEvent::scroll_offset();

        if (offset.y != 0.f)
        {
            camera->viewing_angle(glm::radians(glm::degrees(camera->viewing_angle()) + offset.y));
        }

        if (Engine::KeyboardEvent::just_pressed() == Engine::KEY_V)
            Engine::Window::vsync(!Engine::Window::vsync());

        {
            static bool can_change = true;
            if (Engine::KeyboardEvent::pressed(Engine::KEY_LEFT_CONTROL, Engine::KEY_LEFT_SHIFT, Engine::KEY_M))
            {
                if (can_change)
                {
                    _M_camera_mode = !_M_camera_mode;
                    can_change = false;
                }
            }
            else
            {
                can_change = true;
            }
        }

        if (_M_camera_mode || Engine::MouseEvent::pressed(Engine::MOUSE_BUTTON_MIDDLE))
        {
            if (!_M_hidden_cursor)
            {
                Engine::MouseEvent::relative_mode(true);
                _M_hidden_cursor = true;
            }

            const auto offset = (Engine::MouseEvent::offset() / Engine::Window::size()) * delta_time * speed * -15.f;

            camera->rotate(offset.x, Engine::Constants::OY);
            camera->rotate(offset.y, camera->right_vector());
        }
        else if (_M_hidden_cursor)
        {
            Engine::MouseEvent::relative_mode(false);
            _M_hidden_cursor = false;
            Engine::MouseEvent::position(Engine::Window::size() / 2.f);
        }
    }

    ViewPort::ViewPort()
    {
        _M_framebuffer.gen();

        Engine::TextureParams params;
        params.format = Engine::PixelFormat::RGBA;
        params.pixel_type = Engine::BufferValueType::UNSIGNED_BYTE;
        params.type = Engine::TextureType::Texture_2D;

        _M_colored_texture.create(params);
        _M_colored_texture.gen(Engine::Monitor::size());

        _M_framebuffer.attach_texture(_M_colored_texture, Engine::FrameBufferAttach::COLOR_ATTACHMENT);
        Engine::Window::bind().update_view_port();

        Engine::Camera* camera = new Engine::Camera({0.f, 0.f, 0.f}, glm::radians(75.f));
        camera->max_render_distance(1000.f);
        Resources::scene.add_camera(camera);
        Resources::scene.active_camera(camera);
        Resources::scene.set_as_active_scene();

        params.format = Engine::PixelFormat::DEPTH;
        params.pixel_type = Engine::BufferValueType::UNSIGNED_SHORT;
        _M_depth_texture.create(params);
        _M_depth_texture.gen(Engine::Monitor::size());
        _M_framebuffer.attach_texture(_M_depth_texture, Engine::FrameBufferAttach::DEPTH_ATTACHMENT);
    }


    static Engine::Ray _M_ray;
    static bool _M_enable_picker = false;
    static std::pair<const Engine::DrawableObject*, float> _M_picked = {nullptr, -1.f};

    static void on_render_layer_callback(const Engine::DrawableObject* object, const glm::mat4& matrix)
    {
        if (_M_enable_picker)
        {
            auto ray_cast = object->aabb().apply_model(matrix).intersect(_M_ray);
            if (ray_cast.x > 0.f && ray_cast.x < ray_cast.y)
            {
                if (_M_picked.first == nullptr || _M_picked.second > ray_cast.x)
                {
                    _M_picked.first = object;
                    _M_picked.second = ray_cast.x;
                }
            }
        }

        if (object != dynamic_cast<const Engine::DrawableObject*>(Resources::object_for_properties))
            return;

        if (object == &Resources::scene)
            return;

        object->aabb().render(matrix);
    }

    static void pick_object(const Engine::Point2D& view_pos, const Engine::Size2D& view_size)
    {
        _M_enable_picker = true;

        Engine::Offset2D mouse_offset = 2.f * (Engine::MouseEvent::position() - (view_pos + (view_size / 2.f))) / view_size;
        Engine::Camera* camera = Resources::scene.active_camera().camera;
        mouse_offset.y = -mouse_offset.y;

        float half_fov = camera->viewing_angle() / 2.f;

        const float half_v_side = glm::tan(half_fov);
        const float half_h_side = half_v_side * camera->aspect();

        //Engine::logger->log("{%f %f}", mouse_offset.y, half_v_side);
        _M_ray.origin(camera->position());
        Engine::Vector3D direction = camera->front_vector() + camera->right_vector() * half_h_side * mouse_offset.x +
                                     camera->up_vector() * half_h_side * mouse_offset.y;
        _M_ray.direction(direction);
    }

    void ViewPort::render()
    {
        ImGui::BeginChild("View Port", {0, 0}, true);
        static int FPS = 60;

        if (Engine::Event::frame_number() % 60 == 0)
            FPS = static_cast<int>(1.f / Engine::Event::diff_time());

        ImGui::Text("FPS: %d", FPS);

        auto viewport_size = to_engine_size(ImGui::GetWindowSize());
        ImVec2 pos = ImGui::GetCursorScreenPos();
        Resources::scene.active_camera().camera->aspect(viewport_size.x / viewport_size.y);


        bool cursor_on_panel = Panel::cursor_on_panel();
        if ((_M_hidden_cursor) || cursor_on_panel)
            move_camera();

        if (!_M_hidden_cursor && cursor_on_panel && Engine::MouseEvent::just_pressed() == Engine::MOUSE_BUTTON_LEFT)
        {
            pick_object(to_engine_size(pos), viewport_size);
        }

        Resources::scene.active_camera().update_info();

        auto scale_values = viewport_size / Engine::Monitor::size();
        _M_framebuffer.bind().view_port(Engine::Constants::zero_vector, viewport_size).clear_buffer();
        Resources::object_for_rendering->render(on_render_layer_callback);

        if (_M_enable_picker)
        {
            if (_M_picked.first)
                Resources::object_for_properties =
                        const_cast<Engine::ObjectInstance*>(dynamic_cast<const Engine::ObjectInstance*>(_M_picked.first));
            _M_enable_picker = false;
            _M_picked.first = nullptr;
        }

        Engine::Window::bind().update_view_port();

        auto opengl_texture = _M_colored_texture.internal_id();


        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddImage((void*) opengl_texture, pos, ImVec2(pos.x + viewport_size.x, pos.y + viewport_size.y),
                           ImVec2(0, scale_values.y), ImVec2(scale_values.x, 0));

        ImGui::EndChild();
    }
}// namespace Editor
