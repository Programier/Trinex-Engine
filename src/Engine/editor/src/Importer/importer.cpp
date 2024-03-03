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
    static Vector3D vector_from_assimp_vec(const aiVector3D& vector)
    {
        Vector3D result;
        result.x = vector.x;
        result.y = vector.y;
        result.z = vector.z;
        return result;
    }


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
        Vector<Vector3D> normals;
        Vector<Vector3D> tangents;
        Vector<Vector3D> bitangents;
        Vector<Vector<Vector2D>> uv;
        Vector<uint_t> indices;


        {
            positions.resize(mesh->mNumVertices);

            if (mesh->mNormals)
            {
                normals.resize(mesh->mNumVertices);
            }

            if (mesh->mTangents)
            {
                tangents.resize(mesh->mNumVertices);
            }

            if (mesh->mBitangents)
            {
                bitangents.resize(mesh->mNumVertices);
            }

            for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
            {
                positions[i] = vector_from_assimp_vec(mesh->mVertices[i]);

                if (mesh->mNormals)
                {
                    normals[i] = vector_from_assimp_vec(mesh->mNormals[i]);
                }

                if (mesh->mTangents)
                {
                    tangents[i] = vector_from_assimp_vec(mesh->mTangents[i]);
                }

                if (mesh->mBitangents)
                {
                    bitangents[i] = vector_from_assimp_vec(mesh->mBitangents[i]);
                }
            }

            int uv_slots = AI_MAX_NUMBER_OF_TEXTURECOORDS - 1;

            while (uv_slots >= 0 && mesh->mTextureCoords[uv_slots] == nullptr) --uv_slots;

            ++uv_slots;

            if (uv_slots > 0)
            {
                uv.resize(uv_slots);

                for (int i = 0; i < uv_slots; ++i)
                {
                    auto& slot        = uv[i];
                    auto& assimp_slot = mesh->mTextureCoords[i];

                    if (assimp_slot)
                    {
                        slot.resize(mesh->mNumVertices);

                        for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
                        {
                            slot[j] = vector_from_assimp_vec(assimp_slot[j]);
                        }
                    }
                }
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

        static_mesh->lods.resize(1);
        auto& lod = static_mesh->lods[0];

        if (positions.size() > 0)
        {
            PositionVertexBuffer* position_vertex_buffer = Object::new_instance<PositionVertexBuffer>();
            position_vertex_buffer->buffer               = std::move(positions);
            lod.positions.push_back(position_vertex_buffer);
        }

        if (normals.size() > 0)
        {
            NormalVertexBuffer* normal_vertex_buffer = Object::new_instance<NormalVertexBuffer>();
            normal_vertex_buffer->buffer             = std::move(normals);
            lod.normals.push_back(normal_vertex_buffer);
        }

        if (tangents.size() > 0)
        {
            TangentVertexBuffer* tangent_vertex_buffer = Object::new_instance<TangentVertexBuffer>();
            tangent_vertex_buffer->buffer              = std::move(normals);
            lod.tangents.push_back(tangent_vertex_buffer);
        }

        if (bitangents.size() > 0)
        {
            BinormalVertexBuffer* binormal_vertex_buffer = Object::new_instance<BinormalVertexBuffer>();
            binormal_vertex_buffer->buffer               = std::move(normals);
            lod.binormals.push_back(binormal_vertex_buffer);
        }

        if (uv.size() > 0)
        {
            lod.tex_coords.reserve(uv.size());
            for (auto& uv_slot : uv)
            {
                if (uv_slot.size() > 0)
                {
                    TexCoordVertexBuffer* uv_vertex_buffer = Object::new_instance<TexCoordVertexBuffer>();
                    uv_vertex_buffer->buffer               = std::move(uv_slot);
                    lod.tex_coords.push_back(uv_vertex_buffer);
                }
                else
                {
                    lod.tex_coords.push_back(nullptr);
                }
            }
        }

        if (indices.size() > 0)
        {
            IndexBuffer* index_buffer = Object::new_instance<IndexBuffer>();
            index_buffer->buffer      = std::move(indices);
            lod.indices               = index_buffer;
        }

        static_mesh->bounds = AABB_3Df(vector_from_assimp_vec(mesh->mAABB.mMin), vector_from_assimp_vec(mesh->mAABB.mMax));
        static_mesh->name(mesh->mName.C_Str());
        static_mesh->init_resources();
        package->add_object(static_mesh);
    }

    void import_resource(Package* package, const Path& file)
    {
        info_log("Importer", "Loading resources from file '%s'", file.c_str());

        Assimp::Importer importer;

        unsigned int flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals |
                             aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_GenBoundingBoxes;
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
