#include <Core/class.hpp>
#include <Core/file_manager.hpp>
#include <Core/package.hpp>
#include <GameInitCommandLet.hpp>
#include <Graphics/camera.hpp>
#include <Graphics/framebuffer.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture_2D.hpp>
#include <Graphics/uniform_buffer.hpp>
#include <ImGui/imgui.h>
#include <Window/window.hpp>


namespace Engine
{
    struct Vertex {
        Vector2D pos;
        Vector2D coord;
    };

    static Shader* create_shader(Identifier framebuffer_id = 0)
    {
        PipelineState state;
        state.rasterizer.cull_mode = CullMode::None;
        state.depth_test.enable    = 0;

        ColorBlendAttachmentState color_attachment;
        state.color_blending.blend_attachment.push_back(color_attachment);

        PipelineCreateInfo info;
        ShaderTextureSampler sampler;
        sampler.binding = 0;
        info.texture_samplers.push_back(sampler);
        info.state = &state;

        info.name             = "Test Shader";
        info.vertex_info.size = sizeof(Vertex);
        {
            VertexAtribute attribute;
            attribute.offset = 0;
            attribute.type   = ShaderDataType::type_of<Vector2D>();

            info.vertex_info.attributes.push_back(attribute);

            attribute.offset = offsetof(Vertex, coord);
            info.vertex_info.attributes.push_back(attribute);
        }

        Shader* shader = Object::new_instance<Shader>();

        FileReader reader;
        String dir = "/home/programier/Projects/Shaders/new/";

        {
            reader.open(dir + "output.vm");
            reader.read(info.binaries.vertex, reader.size());

            reader.open(dir + "output.fm");
            reader.read(info.binaries.fragment, reader.size());

            info.text.fragment.emplace_back();
            info.text.vertex.emplace_back();

            reader.open(dir + "output.vert");
            info.text.vertex[0].resize(reader.size() + 1, 0);
            reader.read(info.text.vertex[0].data(), reader.size());

            reader.open(dir + "output.frag");
            info.text.fragment[0].resize(reader.size() + 1, 0);
            reader.read(info.text.fragment[0].data(), reader.size());
        }

        info.framebuffer_usage = framebuffer_id;

        shader->load(info);
        return shader;
    }

    struct ModelUBO {
        Matrix4f model;
    };

    struct CameraUBO {
        Matrix4f projview;
    };

    static Shader* create_framebuffer_shader(Identifier framebuffer_id = 0)
    {
        Shader* result = nullptr;
        PipelineState state;
        state.depth_test.enable    = 0;
        state.rasterizer.cull_mode = CullMode::None;

        ColorBlendAttachmentState color_attachment;
        state.color_blending.blend_attachment.push_back(color_attachment);

        PipelineCreateInfo info;
        ShaderTextureSampler sampler;
        sampler.binding = 2;
        info.texture_samplers.push_back(sampler);
        info.state = &state;

        info.name             = "Test Shader 2";
        info.vertex_info.size = sizeof(Vertex);
        {
            VertexAtribute attribute;
            attribute.offset = 0;
            attribute.type   = ShaderDataType::type_of<Vector2D>();

            info.vertex_info.attributes.push_back(attribute);

            attribute.offset = offsetof(Vertex, coord);
            info.vertex_info.attributes.push_back(attribute);
        }


        FileReader reader;
        String dir = "/home/programier/Projects/Shaders/new/";

        {
            reader.open(dir + "framebuffer.vm");
            reader.read(info.binaries.vertex, reader.size());

            reader.open(dir + "framebuffer.fm");
            reader.read(info.binaries.fragment, reader.size());

            info.text.fragment.emplace_back();
            info.text.vertex.emplace_back();

            reader.open(dir + "framebuffer.vert");
            info.text.vertex[0].resize(reader.size() + 1, 0);
            reader.read(info.text.vertex[0].data(), reader.size());

            reader.open(dir + "framebuffer.frag");
            info.text.fragment[0].resize(reader.size() + 1, 0);
            reader.read(info.text.fragment[0].data(), reader.size());
        }

        info.framebuffer_usage = framebuffer_id;


        ShaderUniformBuffer uniform;
        uniform.binding = 0;
        uniform.name    = "camera_ubo";
        uniform.size    = sizeof(CameraUBO);

        info.uniform_buffers.push_back(uniform);

        uniform.binding = 1;
        uniform.name    = "model_ubo";
        uniform.size    = sizeof(ModelUBO);

        info.uniform_buffers.push_back(uniform);

        info.max_textures_binding_per_frame = 5000 * 4;

        info.state->depth_test.enable       = true;
        info.state->depth_test.write_enable = true;

        result = Object::new_instance<Shader>();
        result->load(info);

        return result;
    }

    static void update_camera(Camera* camera)
    {
        static float speed  = 5.0f;
        float current_speed = speed * Event::diff_time();

        if (KeyboardEvent::pressed(KEY_W))
        {
            camera->move(camera->front_vector() * current_speed);
        }

        if (KeyboardEvent::pressed(KEY_S))
        {
            camera->move(camera->front_vector() * -current_speed);
        }

        if (KeyboardEvent::pressed(KEY_D))
        {
            camera->move(camera->right_vector() * current_speed);
        }

        if (KeyboardEvent::pressed(KEY_A))
        {
            camera->move(camera->right_vector() * -current_speed);
        }

        if (KeyboardEvent::pressed(KEY_SPACE))
        {
            float k = (KeyboardEvent::pressed(KEY_LEFT_SHIFT) ? -1.f : 1.f);
            camera->move(camera->up_vector() * current_speed * k);
        }

        if (KeyboardEvent::just_pressed(KEY_ENTER))
        {
            MouseEvent::relative_mode(!MouseEvent::relative_mode());
        }

        if (MouseEvent::relative_mode())
        {
            auto offset = MouseEvent::offset() / (Window::size());
            camera->rotate(-offset.x, Constants::OY);
            camera->rotate(offset.y, camera->right_vector());
        }
    }

    void GameInit::loop()
    {
        Window window;

        Texture2D color_texture[2];
        Texture2D depth_texture[2];
        {
            TextureCreateInfo& info1 = color_texture[0].resources(true)->info;

            info1.size                 = {1280, 720};
            info1.pixel_type           = PixelType::RGBA;
            info1.pixel_component_type = PixelComponentType::UnsignedByte;
            color_texture[0].create();

            TextureCreateInfo& info2 = color_texture[1].resources(true)->info;

            info2.size                 = {1280, 720};
            info2.pixel_type           = PixelType::RGBA;
            info2.pixel_component_type = PixelComponentType::UnsignedByte;
            color_texture[1].create();

            TextureCreateInfo& depth_info1   = depth_texture[0].resources(true)->info;
            depth_info1.pixel_component_type = PixelComponentType::Depth16;
            depth_info1.pixel_type           = PixelType::Depth;
            depth_info1.size                 = {1280, 720};
            depth_texture[0].create();

            TextureCreateInfo& depth_info2   = depth_texture[1].resources(true)->info;
            depth_info2.pixel_component_type = PixelComponentType::Depth16;
            depth_info2.pixel_type           = PixelType::Depth;
            depth_info2.size                 = {1280, 720};
            depth_texture[1].create();
        }

        FrameBuffer framebuffer;
        {
            FrameBufferCreateInfo info;
            info.size = {1280, 720};

            info.buffers.resize(2);
            FrameBufferAttachment attachment;
            attachment.texture_id = color_texture[0].id();
            info.buffers[0].color_attachments.push_back(attachment);

            attachment.texture_id = color_texture[1].id();
            info.buffers[1].color_attachments.push_back(attachment);

            attachment.texture_id                    = depth_texture[0].id();
            info.buffers[0].depth_stencil_attachment = attachment;

            attachment.texture_id                    = depth_texture[1].id();
            info.buffers[1].depth_stencil_attachment = attachment;


            FrameBufferAttachmentClearData data;
            data.clear_on_bind     = 1;
            data.clear_value.color = ColorClearValue(0.0, 0.0, 0.0, 1.0);

            info.color_clear_data.push_back(data);

            data.clear_value.depth_stencil = DepthStencilClearValue({1.0, 0});
            info.depth_stencil_clear_data  = data;

            framebuffer.create(info);
        }

        Shader* shader             = create_shader();
        Shader* framebuffer_shader = create_framebuffer_shader(framebuffer.id());


        Package* package = Object::find_package("TestResources", true);
        if (package)
        {
            package->load();
        }

        VertexBuffer& vertex_buffer = *package->find_object_checked_in_package<VertexBuffer>("Vertex Buffer");
        IndexBuffer& index_buffer   = *package->find_object_checked_in_package<IndexBuffer>("Index Buffer");
        VertexBuffer& output_vertex_buffer =
                *package->find_object_checked_in_package<VertexBuffer>("Output Vertex Buffer");
        IndexBuffer& output_index_buffer = *package->find_object_checked_in_package<IndexBuffer>("Output Index Buffer");
        Texture2D& texture               = *package->find_object_checked_in_package<Texture2D>("Trinex Texture");


        UniformBuffer<CameraUBO> camera_ubo[2];

        Camera* camera = Object::new_instance<Camera>(Vector3D{0, 0, 0.3});
        Camera* model  = Object::new_instance<Camera>(Vector3D{0, 0, 0.0});

        camera->min_render_distance(0.01).max_render_distance(1000.f);
        camera->viewing_angle(glm::radians(70.f));
        camera->aspect(window.width() / window.height());

        for (int i = 0; i < 2; i++)
        {
            camera_ubo[i].buffer.projview = camera->projview();
            camera_ubo[i].create();
        }


        UniformBuffer<ModelUBO> ubo;

        ubo.buffer.model = glm::translate(glm::rotate(Constants::identity_matrix, glm::radians(90.f), Constants::OX),
                                          Vector3D(0, 0, 0.0));
        ubo.create();


        Camera* current_camera = camera;

        static size_t index = 0;
        while (window.is_open())
        {

            if (MouseEvent::scroll_offset().y != 0)
            {
                float current = texture.anisotropic_filtering() + MouseEvent::scroll_offset().y;
                info_log("Antialiazing: %f", current);
                texture.anisotropic_filtering(current);
            }


            _M_renderer->begin();

            camera_ubo[index].buffer.projview = camera->projview();
            camera_ubo[index].update(0, sizeof(CameraUBO));

            ubo.buffer.model = model->model();
            ubo.update(0, sizeof(ModelUBO));

            framebuffer.bind(index);
            framebuffer_shader->use();
            index_buffer.bind();


            vertex_buffer.bind();
            camera_ubo[index].bind_buffer(0);

            ubo.bind_buffer(1);
            texture.bind(2);

            _M_renderer->draw_indexed(index_buffer.elements_count(), 0);


            window.bind();
            shader->use();
            output_vertex_buffer.bind();
            output_index_buffer.bind();
            color_texture[index].bind();

            _M_renderer->draw_indexed(output_index_buffer.elements_count(), 0);


            ImGuiRenderer::new_frame();

            ImGui::ShowMetricsWindow();
            ImGuiRenderer::render();

            _M_renderer->end();


            Event::poll_events();
            window.swap_buffers();
            update_camera(current_camera);

            //index = (index + 1) % 2;


            if (KeyboardEvent::just_pressed(KEY_G))
            {
                for (Object* object : Object::all_objects())
                {
                    if (object->is_instance_of<VertexBuffer>())
                    {
                        VertexBuffer* buffer = object->instance_cast<VertexBuffer>();

                        auto resources     = buffer->resources(true);
                        auto mapped_memory = buffer->map_memory();
                        resources->resize(mapped_memory.size());

                        std::copy(mapped_memory.begin(), mapped_memory.end(), resources->begin());
                        buffer->unmap_memory();
                    }

                    if (object->is_instance_of<IndexBuffer>())
                    {
                        IndexBuffer* buffer = object->instance_cast<IndexBuffer>();

                        auto resources     = buffer->resources(true);
                        auto mapped_memory = buffer->map_memory();
                        resources->resize(mapped_memory.size());

                        std::copy(mapped_memory.begin(), mapped_memory.end(), resources->begin());
                        buffer->unmap_memory();
                    }
                }
                package->save();
            }


            if (KeyboardEvent::just_pressed(KEY_1))
            {
                current_camera = camera;
            }
            else if (KeyboardEvent::just_pressed(KEY_2))
            {
                current_camera = model;
            }
        }

        _M_renderer->wait_idle();
    }

    int GameInit::execute(int argc, char** argv)
    {
        _M_renderer = Engine::EngineInstance::instance()->renderer();
        Window window;
        window.init({1280, 720}, "Trinex Engine", WindowAttrib::WinResizable);
        ImGuiRenderer::init();

        loop();
        return 0;
    }

    register_class(GameInit, Engine::CommandLet);

}// namespace Engine
