#include <Core/class.hpp>
#include <Core/commandlet.hpp>
#include <Core/engine.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/system.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture_2D.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/event_system.hpp>
#include <Window/window.hpp>


namespace Engine
{


    class HelloTriangleSystem : public Singletone<HelloTriangleSystem, System>
    {
        declare_class(HelloTriangleSystem, System);


    private:
        static HelloTriangleSystem* _M_instance;

        VertexShader* vertex_shader;
        FragmentShader* fragment_shader;
        Pipeline* pipeline;
        VertexBuffer* vertices;
        IndexBuffer* index_buffer;
        Sampler* sampler;
        Texture2D* texture;
        SSBO* ssbo;


    public:
        static void load_code(Shader* shader, const Path& path)
        {
            FileReader reader(FileManager::root_file_manager()->work_dir() / path);
            shader->binary_code.resize(reader.size());
            reader.read(reinterpret_cast<byte*>(shader->binary_code.data()), shader->binary_code.size());
        }


        void create_vertex_shader()
        {
            vertex_shader = Object::new_instance<VertexShader>();
            load_code(vertex_shader, "./shaders/hello_triangle/vertex.vm");

            vertex_shader->attributes.emplace_back();
            vertex_shader->attributes.back().name = "position";
            vertex_shader->attributes.back().rate = VertexAttributeInputRate::Vertex;
            vertex_shader->attributes.back().type = ShaderDataType::type_of<Vector2D>();

            vertex_shader->ssbo.emplace_back();
            vertex_shader->ssbo.back().binding = 2;

            vertex_shader->rhi_create();
        }

        void create_fragment_shader()
        {
            fragment_shader = Object::new_instance<FragmentShader>();
            load_code(fragment_shader, "./shaders/hello_triangle/fragment.fm");
            fragment_shader->samplers.emplace_back();
            fragment_shader->textures.emplace_back();
            fragment_shader->textures.back().binding = 1;


            fragment_shader->rhi_create();
        }

        void create_pipeline()
        {
            pipeline                  = Object::new_instance<Pipeline>();
            pipeline->vertex_shader   = vertex_shader;
            pipeline->fragment_shader = fragment_shader;
            pipeline->render_target   = engine_instance->window();
            pipeline->color_blending.blend_attachment.emplace_back();
            pipeline->rasterizer.cull_mode = CullMode::None;

            pipeline->rhi_create();
        }

        void create_vertex_buffers()
        {
            {
                Vector2D positions[] = {Vector2D(-1.0, -1.0), Vector2D(-1.0, 1.0), Vector2D(1.0, 1.0),
                                        Vector2D(1.0, -1.0)};
                vertices             = Object::new_instance<VertexBuffer>();
                Buffer* buffer       = vertices->resources(true);

                buffer->resize(sizeof(Vector2D) * 4);
                std::memcpy(buffer->data(), positions, buffer->size());

                vertices->rhi_create();
            }
            {
                index_buffer   = Object::new_instance<IndexBuffer>();
                Buffer* buffer = index_buffer->resources(true);

                (*buffer) = {0, 1, 2, 0, 2, 3};
                index_buffer->component(IndexBufferComponent::UnsignedByte);
                index_buffer->rhi_create();
            }
        }

        void create_texture()
        {
            sampler = Object::new_instance<Sampler>();
            sampler->rhi_create();


            Vector<byte> image = {
                    255, 0,   0,   255,//
                    0,   255, 0,   255,//
                    0,   0,   255, 255,//
                    255, 255, 255, 255,//
            };

            texture                      = Object::new_instance<Texture2D>();
            texture->info.base_mip_level = 0;
            texture->info.format         = ColorFormat::R8G8B8A8Unorm;
            texture->info.size           = {2, 2};
            texture->create(image.data());
        }

        void create_ssbo()
        {
            ssbo = Object::new_instance<SSBO>();

            Vector<int> test = {5, 1, 2, 3, 4, 5};
            ssbo->init_size  = sizeof(int) * test.size();
            ssbo->init_data  = reinterpret_cast<const byte*>(test.data());
            ssbo->rhi_create();
        }

        System& create() override
        {
            Super::create();
            EngineSystem::instance()->add_object(this);

            create_vertex_shader();
            create_fragment_shader();
            create_pipeline();
            create_vertex_buffers();
            create_texture();
            create_ssbo();


            return *this;
        }

        void wait() override
        {
            Super::wait();
        }

        System& update(float dt) override
        {
            Super::update(dt);
            engine_instance->api_interface()->begin_render();
            engine_instance->window()->bind();
            pipeline->bind();
            vertices->bind(0, 0);
            index_buffer->bind();

            sampler->bind(0);
            texture->bind(1);
            ssbo->bind(2);

            engine_instance->api_interface()->draw_indexed(6, 0);
            engine_instance->api_interface()->end_render();

            engine_instance->api_interface()->swap_buffer();

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
            engine_instance->create_window();

            EventSystem::init_all();
            System::new_system<HelloTriangleSystem>();

            engine_instance->launch_systems();

            return 0;
        }
    };

    implement_class(HelloTriangle, "");
    implement_default_initialize_class(HelloTriangle);
    implement_class(HelloTriangleSystem, "Engine");
    implement_default_initialize_class(HelloTriangleSystem);
}// namespace Engine
