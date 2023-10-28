#include <Core/file_manager.hpp>
#include <Graphics/g_buffer.hpp>
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
        vertex_shader->attributes.back().name = "position";
        vertex_shader->attributes.back().rate = VertexAttributeInputRate::Vertex;
        vertex_shader->attributes.back().type = ShaderDataType::Vec3;

        vertex_shader->rhi_create();
        return vertex_shader;
    }

    static FragmentShader* create_fragment_shader()
    {
        FragmentShader* fragment_shader = Object::new_instance<FragmentShader>();
        load_code(fragment_shader, "./shaders/hello_triangle/fragment.fm");
        fragment_shader->combined_samplers.emplace_back();
        fragment_shader->combined_samplers.back().binding = 0;

        fragment_shader->rhi_create();
        return fragment_shader;
    }

    static Pipeline* create_pipeline()
    {
        Pipeline* pipeline        = Object::new_instance<Pipeline>();
        pipeline->vertex_shader   = create_vertex_shader();
        pipeline->fragment_shader = create_fragment_shader();
        pipeline->render_pass     = GBuffer::instance()->render_pass;
        pipeline->color_blending.blend_attachment.emplace_back();
        pipeline->rasterizer.cull_mode = CullMode::None;

        pipeline->rhi_create();
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

        return material;
    }

    static void load_positions(PositionBuffer& buffer)
    {
        buffer = {Vector3D(-1.0, -1.0, 0.0), Vector3D(-1.0, 1.0, 0.0), Vector3D(1.0, 1.0, 0.0),
                  Vector3D(1.0, -1.0, 0.0)};

        for(auto& ell : buffer)
        {
            ell /= 2.0f;
        }
    }

    static void load_index_buffer(IndexBuffer::ShortBuffer& buffer)
    {
        buffer = {0, 1, 2, 0, 2, 3};
    }

    static void load_lod_data(StaticMesh::LOD& lod)
    {
        {
            PositionVertexBuffer* positions = Object::new_instance<PositionVertexBuffer>();
            load_positions(positions->buffer);
            positions->rhi_create();

            lod.positions.push_back(positions);

            lod.indices = Object::new_instance<IndexBuffer>();
            lod.indices->setup(IndexBufferComponent::UnsignedShort);
            load_index_buffer(*lod.indices->short_buffer());

            lod.indices->rhi_create();
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
