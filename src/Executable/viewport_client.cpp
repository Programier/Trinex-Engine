#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/file_manager.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Event/event.hpp>
#include <Event/event_data.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_parameters.hpp>
#include <Systems/event_system.hpp>
#include <Systems/mouse_system.hpp>

namespace Engine
{
    class GameViewportClient : public ViewportClient
    {
        declare_class(GameViewportClient, ViewportClient);
        TexCoordVertexBuffer* vertex_buffer;
        Material* material;

        CameraComponent* camera;

        float speed             = 10.f;
        Vector3D move_direction = Vector3D(0.f);
        Vector2D move_offset    = {0, 0};

    public:
        void move(float value, const Event& event)
        {
            const KeyEvent& key_event = event.get<const KeyEvent&>();
            if (key_event.key == Keyboard::Key::W)
            {
                move_direction.z = -value;
            }

            if (key_event.key == Keyboard::Key::S)
            {
                move_direction.z = value;
            }

            if (key_event.key == Keyboard::Key::A)
            {
                move_direction.x = -value;
            }

            if (key_event.key == Keyboard::Key::D)
            {
                move_direction.x = value;
            }
        }

        void on_mouse_move(const Event& event)
        {
            const MouseMotionEvent& data = event.get<const MouseMotionEvent&>();
            move_offset                  = {float(data.xrel), float(data.yrel)};
        }

        ViewportClient& on_bind_to_viewport(class RenderViewport* viewport) override
        {
            vertex_buffer         = Object::new_instance<TexCoordVertexBuffer>();
            vertex_buffer->buffer = {{-1, -1}, {-1, 1}, {1, 1}, {-1, -1}, {1, 1}, {1, -1}};
            vertex_buffer->init_resource();

            material           = Object::new_instance<Material>();
            Pipeline* pipeline = material->pipeline;

            pipeline->vertex_shader->attributes.push_back(VertexShader::Attribute(ColorFormat::R32G32Sfloat));
            pipeline->vertex_shader->binary_code = FileReader("shaders/gradient/vertex.vm").read_buffer();
            pipeline->vertex_shader->text_code   = FileReader("shaders/gradient/vertex.vert").read_string();
            pipeline->local_parameters.update("offset", 0);
            pipeline->has_global_parameters = true;

            pipeline->fragment_shader->binary_code = FileReader("shaders/gradient/fragment.fm").read_buffer();
            pipeline->fragment_shader->text_code   = FileReader("shaders/gradient/fragment.frag").read_string();

            pipeline->depth_test.enable    = false;
            pipeline->rasterizer.cull_mode = CullMode::None;

            pipeline->color_blending.blend_attachment.resize(1);
            pipeline->render_pass = RenderPassType::Window;
            material->postload();


            camera = Object::new_instance<CameraComponent>();


            EventSystem* event_system = EventSystem::new_system<EventSystem>();

            event_system->add_listener(EventType::KeyDown,
                                       std::bind(&GameViewportClient::move, this, 1.0f, std::placeholders::_1));
            event_system->add_listener(EventType::KeyUp, std::bind(&GameViewportClient::move, this, 0.0f, std::placeholders::_1));

            event_system->add_listener(EventType::MouseMotion,
                                       std::bind(&GameViewportClient::on_mouse_move, this, std::placeholders::_1));

            MouseSystem::new_system<MouseSystem>()->relative_mode(true);
            return *this;
        }

        ViewportClient& render(class RenderViewport* viewport) override
        {
            static GlobalShaderParameters params;

            auto view = camera->camera_view();
            params.update(viewport->base_render_target(), nullptr);
            engine_instance->rhi()->push_global_params(params);

            viewport->rhi_bind();

            for (int i = 0; i < 2; i++)
            {
                material->apply();
                vertex_buffer->rhi_bind(0, 0);

                Vector3D offset = {0, 0, -i};
                engine_instance->rhi()->update_local_parameter(&offset, sizeof(offset), 0);
                engine_instance->rhi()->draw(6);
            }

            engine_instance->rhi()->pop_global_params();
            return *this;
        }

        ViewportClient& update(class RenderViewport* viewport, float dt) override
        {
            camera->transform.location += Matrix3f(camera->transform.rotation_matrix()) * move_direction * speed * dt;
            camera->transform.rotation.y -= move_offset.x * dt * 5.f;
            camera->transform.rotation.x += move_offset.y * dt * 5.f;

            move_offset = {0, 0};

            auto size            = viewport->size();
            camera->aspect_ratio = size.x / size.y;
            camera->transform.update(camera);
            return *this;
        }
    };

    implement_engine_class_default_init(GameViewportClient);
}// namespace Engine
