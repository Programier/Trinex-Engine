#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/engine.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/system.hpp>
#include <Graphics/camera.hpp>
#include <Graphics/g_buffer.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture_2D.hpp>
#include <Systems/event_system.hpp>
#include <Systems/render_system.hpp>
#include <Window/window.hpp>
#include <imgui.h>


namespace Engine
{
    extern void load_code(Shader* shader, const Path& path);

    Camera* camera    = nullptr;
    int output_buffer = 0;

    class HelloTriangleSystem : public Singletone<HelloTriangleSystem, System>
    {
        declare_class(HelloTriangleSystem, System);


    private:
        VertexShader* out_vertex_shader;
        FragmentShader* out_fragment_shader;
        Pipeline* out_pipeline;
        TexCoordVertexBuffer* vertices;
        IndexBuffer* index_buffer;
        Sampler* sampler;
        StaticMesh* mesh;

        Vector<UniformBuffer*> models;
        Vector<Matrix4f> matrices;


    public:
        void create_vertex_shader()
        {

            out_vertex_shader = Object::new_instance<VertexShader>();
            load_code(out_vertex_shader, "./shaders/resolve_gbuffer/vertex.v");
            out_vertex_shader->attributes.emplace_back();
            out_vertex_shader->attributes.back().name   = "position";
            out_vertex_shader->attributes.back().rate   = VertexAttributeInputRate::Vertex;
            out_vertex_shader->attributes.back().format = ColorFormat::R32G32Sfloat;

            out_vertex_shader->init_resource();
        }

        void create_fragment_shader()
        {
            out_fragment_shader = Object::new_instance<FragmentShader>();
            load_code(out_fragment_shader, "./shaders/resolve_gbuffer/fragment.f");
            out_fragment_shader->combined_samplers.emplace_back();
            out_fragment_shader->combined_samplers.back().location = {0, 0};

            out_fragment_shader->init_resource();
        }

        void create_pipeline()
        {
            out_pipeline                  = Object::new_instance<Pipeline>();
            out_pipeline->vertex_shader   = out_vertex_shader;
            out_pipeline->fragment_shader = out_fragment_shader;
            out_pipeline->render_pass     = engine_instance->window()->render_pass;
            out_pipeline->color_blending.blend_attachment.emplace_back();
            out_pipeline->rasterizer.cull_mode = CullMode::None;
            out_pipeline->depth_test.enable    = false;

            out_pipeline->init_resource();
        }

        void create_vertex_buffers()
        {
            {
                // Using TexCoord vertex buffer, because this buffer has vec2 type
                TexCoordVertexBuffer* vertex_buffer = Object::new_instance<TexCoordVertexBuffer>();
                vertex_buffer->buffer               = {Vector2D(-1.0, -1.0), Vector2D(-1.0, 1.0), Vector2D(1.0, 1.0),
                                                       Vector2D(1.0, -1.0)};
                vertex_buffer->init_resource();
                vertices = vertex_buffer;
            }
            {
                index_buffer = Object::new_instance<IndexBuffer>();
                index_buffer->setup(IndexBufferComponent::UnsignedByte);
                (*index_buffer->byte_buffer()) = {0, 1, 2, 0, 2, 3};
                index_buffer->init_resource();
            }
        }

        void create_sampler()
        {
            sampler = Object::new_instance<Sampler>();
            sampler->init_resource();
        }

        void create_models()
        {
            int_t size = 2;

            models.reserve(size * size * size);
            matrices.reserve(size * size * size);


            for (int_t x = 0; x < size; x++)
            {
                for (int_t y = 0; y < size; y++)
                {
                    for (int_t z = 0; z < size; z++)
                    {
                        glm::mat4 model =
                                glm::mat4(1.0);//= glm::translate(glm::mat4(1.0f), glm::vec3(x * 3, y * 3, z * 3));
                        matrices.push_back(model);
                        UniformBuffer* ubo = Object::new_instance<UniformBuffer>();
                        ubo->init_size     = sizeof(model);
                        ubo->init_data     = reinterpret_cast<const byte*>(&matrices.back());
                        ubo->init_resource();
                        models.push_back(ubo);
                    }
                }
            }
        }

        System& create() override
        {
            Super::create();
            System::new_system<RenderSystem>()->register_subsystem(this);

            create_vertex_shader();
            create_fragment_shader();
            create_pipeline();
            create_vertex_buffers();
            create_sampler();

            extern StaticMesh* load_test_object();
            mesh = load_test_object();

            extern Camera* create_test_camera();
            camera = create_test_camera();
            create_models();

            return *this;
        }

        HelloTriangleSystem& wait() override
        {
            Super::wait();
            return *this;
        }


        HelloTriangleSystem& render_gbuffer(float dt)
        {
            GBuffer* rt             = GBuffer::instance();
            rt->global_ubo.projview = camera->projview();
            rt->global_ubo.dt       = dt;

            rt->rhi_bind();
            engine_instance->rhi()->push_debug_stage(__FUNCTION__, Colors::Red);
            for (UniformBuffer* ubo : models)
            {
                mesh->lods[0].render();
                ubo->rhi_bind({2, 0});
                engine_instance->rhi()->draw_indexed(mesh->lods[0].indices->elements_count(), 0);
            }
            engine_instance->rhi()->pop_debug_stage();
            return *this;
        }

        HelloTriangleSystem& render_output()
        {
            engine_instance->window()->rhi_bind();
            engine_instance->rhi()->push_debug_stage(__FUNCTION__, Colors::Green);

            out_pipeline->rhi_bind();
            vertices->rhi_bind(0);
            index_buffer->rhi_bind();

            (GBuffer::instance()->current_frame()->color_attachments)[output_buffer].ptr()->bind_combined(sampler, 0);

            engine_instance->rhi()->draw_indexed(6, 0);
            engine_instance->rhi()->pop_debug_stage();


            return *this;
        }

        HelloTriangleSystem& render_ui(float dt)
        {

            return *this;
        }

        System& update(float dt) override
        {
            Super::update(dt);
            camera->update(dt);


            int_t size = 1;

            size_t index = 0;

            for (int_t x = 0; x < size; x++)
            {
                for (int_t y = 0; y < size; y++)
                {
                    for (int_t z = 0; z < size; z++)
                    {
                        if (x + y + z != 0)
                        {
                            auto& ubo  = models[index];
                            auto& data = matrices[index];

                            data = glm::rotate(data, dt, {x, y, z});
                            ubo->rhi_update(0, sizeof(data), reinterpret_cast<const byte*>(&data));
                        }

                        index++;
                    }
                }
            }

            render_gbuffer(dt).render_output().render_ui(dt);

            return *this;
        }


        System& shutdown() override
        {
            out_fragment_shader->deferred_destroy();
            Super::shutdown();
            return *this;
        }


        friend class Singletone<HelloTriangleSystem, System>;
    };

    implement_class(HelloTriangleSystem, "Engine");
    implement_default_initialize_class(HelloTriangleSystem);
}// namespace Engine
