#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/system.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>
#include <Systems/render_system.hpp>
#include <Window/window.hpp>

namespace Engine
{

    void load_code(Shader* shader, const Path& path);

    class ExampleWindow : public Singletone<ExampleWindow, System>
    {
        declare_class(ExampleWindow, System);
        Pipeline* _M_pipeline;
        PositionVertexBuffer* vertices;
        IndexBuffer* indices;
        ColorVertexBuffer* colors;

    public:
        void create_pipeline()
        {
            _M_pipeline = new Pipeline();

            VertexShader* vertex_shader = new VertexShader();
            load_code(vertex_shader, "./shaders/example_window/vertex.v");

            {
                vertex_shader->attributes.emplace_back();
                auto& attribute  = vertex_shader->attributes.back();
                attribute.count  = 1;
                attribute.format = ColorFormat::R32G32B32Sfloat;
                attribute.rate   = VertexAttributeInputRate::Vertex;
            }
            {
                vertex_shader->attributes.emplace_back();
                auto& attribute  = vertex_shader->attributes.back();
                attribute.count  = 1;
                attribute.format = ColorFormat::R8G8B8A8Unorm;
                attribute.rate   = VertexAttributeInputRate::Vertex;
            }

            vertex_shader->init_resource();

            FragmentShader* fragment_shader = new FragmentShader();
            load_code(fragment_shader, "./shaders/example_window/fragment.f");
            fragment_shader->init_resource();
            _M_pipeline->vertex_shader   = vertex_shader;
            _M_pipeline->fragment_shader = fragment_shader;
            _M_pipeline->render_pass     = Window::instance()->render_pass;

            _M_pipeline->color_blending.blend_attachment.emplace_back();
            _M_pipeline->rasterizer.cull_mode = CullMode::None;
            _M_pipeline->init_resource();

            // Create vertex buffer
            vertices         = new PositionVertexBuffer();
            vertices->buffer = {
                    Vector3D(0.0, -0.5, 0.0),
                    Vector3D(0.5, 0.5, 0.0),
                    Vector3D(-0.5, 0.5, 0.0),
            };

            vertices->init_resource();

            colors         = new ColorVertexBuffer();
            colors->buffer = {
                    Colors::Red * 255.f,
                    Colors::Green * 255.f,
                    Colors::Blue * 255.f,
            };

            colors->init_resource();


            indices = new IndexBuffer();
            indices->setup(IndexBufferComponent::UnsignedByte);
            (*indices->byte_buffer()) = {0, 1, 2};

            indices->init_resource();
        }

        ExampleWindow& create() override
        {
            Super::create();
            new_system<RenderSystem>()->register_subsystem(this);

            create_pipeline();
            return *this;
        }

        ExampleWindow& update(float dt) override
        {
            Super::update(dt);

            String FPS     = std::to_string(1.0 / dt);
            Window* window = engine_instance->window();

            window->rhi_bind();
            _M_pipeline->rhi_bind();
            vertices->rhi_bind(0, 0);
            colors->rhi_bind(1, 0);
            indices->rhi_bind();
            engine_instance->rhi()->draw_indexed(3, 0);

            window->title(FPS);


            return *this;
        }
    };

    implement_engine_class_default_init(ExampleWindow);
}// namespace Engine
