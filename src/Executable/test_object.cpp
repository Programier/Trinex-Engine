#include <Core/colors.hpp>
#include <Core/file_manager.hpp>
#include <Graphics/g_buffer.hpp>
#include <Graphics/global_uniform_buffer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/shader.hpp>

namespace Engine
{

    static void load_code(Shader* shader, const Path& path)
    {
        FileReader reader(FileManager::root_file_manager()->work_dir() / path);
        shader->binary_code.resize(reader.size());
        reader.read(reinterpret_cast<byte*>(shader->binary_code.data()), shader->binary_code.size());
    }

    static VertexShader* create_vertex_shader()
    {
        class VertexShader* vertex_shader = Object::new_instance<VertexShader>();
        load_code(vertex_shader, "./shaders/hello_triangle/vertex.vm");

        vertex_shader->attributes.emplace_back();
        vertex_shader->attributes.back().name   = "position";
        vertex_shader->attributes.back().rate   = VertexAttributeInputRate::Vertex;
        vertex_shader->attributes.back().format = ColorFormat::R32G32B32Sfloat;

        vertex_shader->attributes.emplace_back();
        vertex_shader->attributes.back().name   = "color";
        vertex_shader->attributes.back().rate   = VertexAttributeInputRate::Vertex;
        vertex_shader->attributes.back().format = ColorFormat::R8G8B8A8Unorm;

        vertex_shader->init_global_ubo({0, 0});

        vertex_shader->init_resource();
        return vertex_shader;
    }

    static FragmentShader* create_fragment_shader()
    {
        FragmentShader* fragment_shader = Object::new_instance<FragmentShader>();
        load_code(fragment_shader, "./shaders/hello_triangle/fragment.fm");


        fragment_shader->init_global_ubo({1, 0});
        fragment_shader->init_resource();
        return fragment_shader;
    }

    static Pipeline* create_pipeline()
    {
        Pipeline* pipeline        = Object::new_instance<Pipeline>();
        pipeline->vertex_shader   = create_vertex_shader();
        pipeline->fragment_shader = create_fragment_shader();
        pipeline->render_pass     = GBuffer::instance()->render_pass;

        for (int i = 0; i < 4; i++) pipeline->color_blending.blend_attachment.emplace_back();
        pipeline->rasterizer.cull_mode = CullMode::None;

        pipeline->init_resource();
        return pipeline;
    }


    static Material* create_object_material()
    {
        Material* material = Object::new_instance<Material>();
        material->pipeline(create_pipeline());


        VertexBufferStream stream;
        stream.stream   = 0;
        stream.semantic = VertexBufferSemantic::Position;
        material->push_stream(stream);

        stream.stream   = 1;
        stream.semantic = VertexBufferSemantic::Color;
        material->push_stream(stream);

        return material;
    }

    static void load_positions(PositionVertexBuffer::BufferType& buffer)
    {
        buffer = {Vector3D(-1.0, -1.0, 1.0),  Vector3D(-1.0, 1.0, 1.0),
                  Vector3D(1.0, 1.0, 1.0),    Vector3D(1.0, -1.0, 1.0),

                  Vector3D(-1.0, -1.0, -1.0), Vector3D(-1.0, 1.0, -1.0),
                  Vector3D(1.0, 1.0, -1.0),   Vector3D(1.0, -1.0, -1.0)};
    }

    ByteColor convert(const Color& color)
    {
        return {color.x * 255.f, color.y * 255.f, color.z * 255.f, color.w * 255.f};
    }

    static void load_color(ColorVertexBuffer::BufferType& buffer)
    {
        buffer = {convert(Colors::Red),       convert(Colors::Green),        convert(Colors::Blue),
                  convert(Colors::White),     convert(Colors::DarkSeaGreen), convert(Colors::Yellow),
                  convert(Colors::LawnGreen), convert(Colors::Chocolate)};
    }


    static void load_index_buffer(IndexBuffer::ShortBuffer& buffer)
    {
        buffer = {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 0, 4, 1, 4, 5, 1,
                  1, 5, 2, 5, 6, 2, 0, 4, 3, 4, 7, 3, 2, 6, 7, 2, 7, 3};
    }

    static void load_lod_data(StaticMesh::LOD& lod)
    {
        {
            PositionVertexBuffer* positions = Object::new_instance<PositionVertexBuffer>();
            load_positions(positions->buffer);
            positions->init_resource();

            ColorVertexBuffer* colors = Object::new_instance<ColorVertexBuffer>();
            load_color(colors->buffer);
            colors->init_resource();


            lod.positions.push_back(positions);
            lod.color.push_back(colors);

            lod.indices = Object::new_instance<IndexBuffer>();
            lod.indices->setup(IndexBufferComponent::UnsignedShort);
            load_index_buffer(*lod.indices->short_buffer());

            lod.indices->init_resource();
        }
    }

    StaticMesh* load_test_object()
    {
        StaticMesh* mesh = Object::new_instance<StaticMesh>();
        mesh->lods.resize(1);
        StaticMesh::LOD& lod = mesh->lods[0];
        load_lod_data(lod);

        lod.material = create_object_material();
        return mesh;
    }
}// namespace Engine
