#include <Core/logger.hpp>
#include <Core/package.hpp>
#include <Importer/importer.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <Graphics/mesh.hpp>
#include <Graphics/pipeline_buffers.hpp>


namespace Engine::Importer
{

    static void load_static_meshes(Package* package, const aiScene* scene, const aiMesh* mesh)
    {
        if (package->contains_object(mesh->mName.C_Str()))
        {
            error_log("Importer", "Cannot load mesh '%s', because package '%s' already contains object '%'", mesh->mName.C_Str(),
                      package->name().c_str(), mesh->mName.C_Str());
            return;
        }

        StaticMesh* static_mesh = Object::new_instance<StaticMesh>();


        Vector<Vector3D> positions;
        Vector<uint> indices;

        // Generate position buffer
        {
            positions.resize(mesh->mNumVertices);
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
            {
                auto vertex = mesh->mVertices[i];
                auto& out   = positions[i];

                out.x = vertex.x;
                out.y = vertex.y;
                out.z = vertex.z;
            }
        }

        // Generate indices
        {
            indices.reserve(mesh->mNumFaces * 3);
            for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
            {
                auto& face = mesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; ++j)
                {
                    indices.push_back(face.mIndices[j]);
                }
            }
        }


        PositionVertexBuffer* position_vertex_buffer = Object::new_instance<PositionVertexBuffer>();
        position_vertex_buffer->buffer               = std::move(positions);
        static_mesh->lods.resize(1);
        auto& lod = static_mesh->lods[0];
        lod.positions.push_back(position_vertex_buffer);

        if (indices.size() > 0)
        {
            IndexBuffer* index_buffer = Object::new_instance<IndexBuffer>();
            index_buffer->setup(IndexBufferComponent::UnsignedInt);
            *index_buffer->int_buffer() = std::move(indices);
            lod.indices                 = index_buffer;
        }

        static_mesh->name(mesh->mName.C_Str());
        static_mesh->init_resources();
        package->add_object(static_mesh);
    }

    void import_resource(Package* package, const Path& file)
    {
        info_log("Importer", "Loading resources from file '%s'", file.c_str());

        Assimp::Importer importer;

        unsigned int flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals |
                             aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;
        const aiScene* scene = importer.ReadFile(file.c_str(), flags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            error_log("Importer", "Failed to load resource: %s", importer.GetErrorString());
            return;
        }

        for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
        {
            const aiMesh* mesh = scene->mMeshes[i];

            if (mesh->mNumBones == 0)
            {
                load_static_meshes(package, scene, mesh);
            }
        }

        importer.FreeScene();
    }
}// namespace Engine::Importer
