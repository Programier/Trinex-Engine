#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/engine.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/system.hpp>
#include <Graphics/g_buffer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture_2D.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/event_system.hpp>
#include <Window/window.hpp>
#include <Graphics/camera.hpp>


namespace Engine
{
    class HelloTriangleSystem : public Singletone<HelloTriangleSystem, System>
    {
        declare_class(HelloTriangleSystem, System);


    private:
        static HelloTriangleSystem* _M_instance;
        VertexShader* out_vertex_shader;
        FragmentShader* out_fragment_shader;
        Pipeline* out_pipeline;
        TexCoordVertexBuffer* vertices;
        IndexBuffer* index_buffer;
        Sampler* sampler;
        class Camera* camera;

        StaticMesh* mesh;


    public:
        static void load_code(Shader* shader, const Path& path)
        {
            FileReader reader(FileManager::root_file_manager()->work_dir() / path);
            shader->binary_code.resize(reader.size());
            reader.read(reinterpret_cast<byte*>(shader->binary_code.data()), shader->binary_code.size());
        }

        void create_vertex_shader()
        {

            out_vertex_shader = Object::new_instance<VertexShader>();
            load_code(out_vertex_shader, "./shaders/resolve_gbuffer/vertex.vm");
            out_vertex_shader->attributes.emplace_back();
            out_vertex_shader->attributes.back().name = "position";
            out_vertex_shader->attributes.back().rate = VertexAttributeInputRate::Vertex;
            out_vertex_shader->attributes.back().type = ShaderDataType::Vec2;

            out_vertex_shader->rhi_create();
        }

        void create_fragment_shader()
        { 
            out_fragment_shader = Object::new_instance<FragmentShader>();
            load_code(out_fragment_shader, "./shaders/resolve_gbuffer/fragment.fm");
            out_fragment_shader->combined_samplers.emplace_back();
            out_fragment_shader->combined_samplers.back().binding = 0;

            out_fragment_shader->rhi_create();
        }

        void create_pipeline()
        {
            out_pipeline                  = Object::new_instance<Pipeline>();
            out_pipeline->vertex_shader   = out_vertex_shader;
            out_pipeline->fragment_shader = out_fragment_shader;
            out_pipeline->render_pass     = engine_instance->window()->render_pass;
            out_pipeline->color_blending.blend_attachment.emplace_back();
            out_pipeline->rasterizer.cull_mode = CullMode::None;

            out_pipeline->rhi_create();
        }

        void create_vertex_buffers()
        {
            {
                // Using TexCoord vertex buffer, because this buffer has vec2 type
                TexCoordVertexBuffer* vertex_buffer = Object::new_instance<TexCoordVertexBuffer>();
                vertex_buffer->buffer               = {Vector2D(-1.0, -1.0), Vector2D(-1.0, 1.0), Vector2D(1.0, 1.0),
                                                       Vector2D(1.0, -1.0)};
                vertex_buffer->rhi_create();
                vertices = vertex_buffer;
            }
            {
                index_buffer = Object::new_instance<IndexBuffer>();
                index_buffer->setup(IndexBufferComponent::UnsignedByte);
                (*index_buffer->byte_buffer()) = {0, 1, 2, 0, 2, 3};
                index_buffer->rhi_create();
            }
        }

        void create_sampler()
        {
            sampler = Object::new_instance<Sampler>();
            sampler->rhi_create();
        }

        System& create() override
        {
            Super::create();
            EngineSystem::instance()->add_object(this);

            create_vertex_shader();
            create_fragment_shader();
            create_pipeline();
            create_vertex_buffers();
            create_sampler();

            extern StaticMesh* load_test_object();
            mesh = load_test_object();

            extern Camera* create_test_camera();
            camera = create_test_camera();


            return *this;
        }

        void wait() override
        {
            Super::wait();
        }


        HelloTriangleSystem& render_gbuffer()
        {
            GBuffer::instance()->bind();
            engine_instance->rhi()->push_debug_stage(__FUNCTION__, Colors::Red);
            mesh->lods[0].render();
            engine_instance->rhi()->pop_debug_stage();
            return *this;
        }

        HelloTriangleSystem& render_output()
        {
            engine_instance->window()->bind();
            engine_instance->rhi()->push_debug_stage(__FUNCTION__, Colors::Green);

            out_pipeline->rhi_bind();
            vertices->rhi_bind(0, 0);
            index_buffer->rhi_bind();

            GBuffer::instance()->albedo.ptr()->bind_combined(sampler, 0, 0);

            engine_instance->rhi()->draw_indexed(6, 0);
            engine_instance->rhi()->pop_debug_stage();
            engine_instance->rhi()->end_render();


            return *this;
        }

        System& update(float dt) override
        {
            Super::update(dt);
            camera->update(dt);

            engine_instance->rhi()->begin_render();
            render_gbuffer().render_output();
            engine_instance->rhi()->swap_buffer();

            return *this;
        }


        System& shutdown() override
        {
            Super::shutdown();
            return *this;
        }


        friend class Singletone<HelloTriangleSystem, System>;
    };

    HelloTriangleSystem* HelloTriangleSystem::_M_instance = nullptr;

    class HelloTriangle : public CommandLet
    {
        declare_class(HelloTriangle, CommandLet);

    public:
        int_t execute(int_t argc, char** argv) override
        {
            info_log("HelloTriangle", "Start");
            EventSystem::init_all();
            System::new_system<HelloTriangleSystem>();

            return engine_instance->launch_systems();
        }
    };


    implement_class(HelloTriangle, "");
    implement_default_initialize_class(HelloTriangle);
    implement_class(HelloTriangleSystem, "Engine");
    implement_default_initialize_class(HelloTriangleSystem);
}// namespace Engine
