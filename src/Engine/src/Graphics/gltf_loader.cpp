#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/gltf_loader.hpp>
#include <tiny_gltf.h>

namespace Engine::GLTF
{

    /*
            {"position", ShaderDataType::type_of<Vector2D>()},
            {"tex_coord", ShaderDataType::type_of<Vector2D>()},
            {"color", ShaderDataType::type_of<Vector4D>()},
            {"normal", ShaderDataType::type_of<Vector3D>()},
            {"tangent", ShaderDataType::type_of<Vector3D>()},
            {"binormal", ShaderDataType::type_of<Vector3D>()},
    */

    static Map<String, VertexBufferSemantic> attribute_semantic = {
            {"POSITION", VertexBufferSemantic::Position},
            {"NORMAL", VertexBufferSemantic::Normal},
            {"TEXCOORD", VertexBufferSemantic::TexCoord},
    };

    static void split_attribute(const String& name, String& out_name, byte index)
    {
        auto it = name.find("_");
        if (it == String::npos)
        {
            out_name = name;
            index    = 0;
            return;
        }

        out_name = name.substr(0, it);
        index    = static_cast<byte>(std::stoi(name.substr(it + 1, name.length() - it - 1)));
    }

    static size_t calculate_vertex_info(const tinygltf::Model& model, const tinygltf::Primitive& primitive,
                                        MeshComponentSemanticInfo& info)
    {
        size_t count = 0;

        for (auto& attribute : primitive.attributes)
        {
            String out_name;
            byte index = 0;

            split_attribute(attribute.first, out_name, index);
            auto it = attribute_semantic.find(out_name);

            if (it == attribute_semantic.end())
                continue;

            index += 1;

            MeshComponentSemanticEntry& entry = info.entry_of(it->second);
            entry.count                       = std::max(entry.count, index);

            const tinygltf::Accessor& accessor = model.accessors[attribute.second];
            count                              = std::max(count, accessor.count);
        }

        return count;
    }

    static bool is_dynamic_MeshComponent(const tinygltf::Primitive& primitive)
    {
        for (auto& pair : primitive.attributes)
        {
            if (pair.first.starts_with("JOINTS") || pair.first.starts_with("BLENDWEIGHT"))
                return true;
        }

        return false;
    }

    template<typename Type>
    static void load_index_buffer(const void* _gltf_indeces, IndexBuffer* index_buffer, IndexBufferComponent component,
                                  size_t count)
    {
        const Type* indices_ptr = reinterpret_cast<const Type*>(_gltf_indeces);
        auto index_resources    = index_buffer->resources(true);
        index_resources->resize(count * sizeof(Type), 0);
        index_buffer->component(component);

        Type* index_data = reinterpret_cast<Type*>(index_resources->data());
        std::copy(indices_ptr, indices_ptr + count, index_data);
    }

    static StaticMeshComponent* load_static_MeshComponent(const tinygltf::Model& model,
                                                          const tinygltf::Primitive& primitive, const String& name)
    {
        if (primitive.indices < 0)
        {
            error_log("GLTF: Cannot load object without indices array!");
            return nullptr;
        }

        StaticMeshComponent* MeshComponent = Object::new_instance_named<StaticMeshComponent>(name);
        MeshComponent->lods.emplace_back();

        size_t vertices = calculate_vertex_info(model, primitive, MeshComponent->info);

        {

            VertexBuffer* vertex_buffer = &MeshComponent->lods[0].vertex_buffer;

            auto vertex_resources = vertex_buffer->resources(true);
            vertex_resources->resize(vertices * MeshComponent->info.vertex_size(), 0);
        }

        for (auto& attribute : primitive.attributes)
        {
            String out_name;
            byte index = 0;

            split_attribute(attribute.first, out_name, index);

            auto semantic_it = attribute_semantic.find(out_name);

            if (semantic_it == attribute_semantic.end())
                continue;

            const tinygltf::Accessor& accessor      = model.accessors[attribute.second];
            const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer          = model.buffers[buffer_view.buffer];

            const float* data =
                    reinterpret_cast<const float*>(&(buffer.data[accessor.byteOffset + buffer_view.byteOffset]));

            size_t vertex_size                         = MeshComponent->info.vertex_size();
            MeshComponentSemanticEntry& semantic_entry = MeshComponent->info.entry_of(semantic_it->second);
            size_t floats_count                        = semantic_entry.type_size() / sizeof(float);
            size_t vertex_offset = MeshComponent->info.semantic_offset(semantic_it->second, index);

            for (size_t k = 0; k < accessor.count; k++)
            {
                size_t resource_offset    = (k * vertex_size) + vertex_offset;
                float* MeshComponent_data = reinterpret_cast<float*>(
                        MeshComponent->lods[0].vertex_buffer.resources()->data() + resource_offset);

                for (size_t i = 0; i < floats_count; i++)
                {
                    MeshComponent_data[i] = data[(k * floats_count) + i];
                }
            }
        }

        // Process MeshComponent indices
        {
            IndexBuffer* index_buffer = &MeshComponent->lods[0].index_buffer;


            const tinygltf::Accessor& accessor      = model.accessors[primitive.indices];
            const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer          = model.buffers[buffer_view.buffer];


            const void* indices_ptr = (&(buffer.data[accessor.byteOffset + buffer_view.byteOffset]));

            if (accessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT)
            {
                load_index_buffer<Engine::uint_t>(indices_ptr, index_buffer, IndexBufferComponent::UnsignedInt,
                                                  accessor.count);
            }
            else if (accessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE)
            {
                load_index_buffer<Engine::byte>(indices_ptr, index_buffer, IndexBufferComponent::UnsignedByte,
                                                accessor.count);
            }
            else if (accessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)
            {
                load_index_buffer<Engine::ushort_t>(indices_ptr, index_buffer, IndexBufferComponent::UnsignedShort,
                                                    accessor.count);
            }
        }

        return MeshComponent;
    }


    static void load_primitive(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const String& name,
                               Vector<MeshComponent*>& result)
    {
        if (is_dynamic_MeshComponent(primitive))
        {
            // TODO
        }
        else
        {
            result.push_back(load_static_MeshComponent(model, primitive, name));
        }
    }

    static void load_mesh_components(const tinygltf::Model& model, const tinygltf::Mesh& mesh,
                                     Vector<MeshComponent*>& result)
    {
        Index i = 1;

        if (mesh.primitives.size() == 1)
        {
            load_primitive(model, mesh.primitives[0], mesh.name, result);
        }
        else
        {
            for (auto& primitive : mesh.primitives)
            {
                load_primitive(model, primitive, Strings::format("{} {}", mesh.name, i++), result);
            }
        }
    }

    Vector<MeshComponent*> load_MeshComponentes(const String& path)
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;

        std::string err;
        std::string warn;

        bool success = path.ends_with(".gltf") ? loader.LoadASCIIFromFile(&model, &err, &warn, path)
                                               : loader.LoadBinaryFromFile(&model, &err, &warn, path);

        if (!warn.empty())
        {
            logger->warning("GLTF: Warning: %s", warn.c_str());
        }

        if (!err.empty())
        {
            logger->error("GLTF: Error: %s", err.c_str());
            return {};
        }

        if (!success)
        {
            logger->error("GLTF: Failed to load '%s'", path.c_str());
            return {};
        }

        Vector<MeshComponent*> result;

        for (auto& MeshComponent : model.meshes)
        {
            load_mesh_components(model, MeshComponent, result);
        }

        return result;
    }
}// namespace Engine::GLTF
