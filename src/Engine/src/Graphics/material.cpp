#include "Core/engine_types.hpp"
#include "Core/object.hpp"
#include <Core/class.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh_component.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture.hpp>


namespace Engine
{
    bool MaterialLayout::operator==(const MaterialLayout& layout) const
    {
        if (vertex_size != layout.vertex_size || offsets.size() != layout.offsets.size())
            return false;

        Index i = 0;
        for (auto offset : offsets)
        {
            if (offset != layout.offsets[i++])
                return false;
        }

        return true;
    }


    static const Map<String, ShaderDataType> semantic_types = {
            {"position", ShaderDataType::type_of<Vector3D>()},
            {"tex_coord", ShaderDataType::type_of<Vector2D>()},
            {"color", ShaderDataType::type_of<Vector4D>()},
            {"normal", ShaderDataType::type_of<Vector3D>()},
            {"tangent", ShaderDataType::type_of<Vector3D>()},
            {"binormal", ShaderDataType::type_of<Vector3D>()},
            {"blend_weight", ShaderDataType::type_of<Vector4D>()},
            {"blend_indices", ShaderDataType::type_of<IntVector4D>()},
    };

    static const Map<String, VertexBufferSemantic> semantic_names = {
            {"position", VertexBufferSemantic::Position},
            {"tex_coord", VertexBufferSemantic::TexCoord},
            {"color", VertexBufferSemantic::Color},
            {"normal", VertexBufferSemantic::Normal},
            {"tangent", VertexBufferSemantic::Tangent},
            {"binormal", VertexBufferSemantic::Binormal},
            {"blend_weight", VertexBufferSemantic::BlendWeight},
            {"blend_indices", VertexBufferSemantic::BlendIndices},
    };

    static bool find_type(const String& semantic_name, ShaderDataType& type, byte& index,
                          VertexBufferSemantic& semantic)
    {
        if (!semantic_name.starts_with("in_"))
        {
            return false;
        }

        auto it = semantic_name.find_last_of("_");

        if (it == String::npos || it < 3)
        {
            return false;
        }


        String name      = semantic_name.substr(3, it - 3);
        auto semantic_it = semantic_types.find(name);
        if (semantic_it == semantic_types.end())
            return false;
        semantic = semantic_names.find(name)->second;
        type     = semantic_it->second;

        name  = semantic_name.substr(it + 1, semantic_name.length() - it - 1);
        index = static_cast<byte>(std::stoi(name));
        return true;
    }


    MaterialApplier* Material::create_material_applier(MeshComponent* mesh)
    {
        if (mesh == nullptr || _M_resources == nullptr)
            return nullptr;

        VertexBufferInfo& vertex_info = _M_resources->vertex_info;


        MaterialLayout layout;
        layout.vertex_size = mesh->semantic_info().vertex_size();
        vertex_info.size   = layout.vertex_size;

        for (auto& entry : vertex_info.attributes)
        {
            byte index;
            VertexBufferSemantic semantic;
            if (!find_type(entry.name, entry.type, index, semantic))
            {
                throw EngineException("Material: Undefined semantic");
            }

            entry.offset = mesh->semantic_info().semantic_offset(semantic, index);
            if (entry.offset == Constants::index_none)
            {
                error_log("Material: Cannot create material applier for mesh '%s'. "
                          "Cannot find semantic '%s' in mesh info!",
                          mesh->name().c_str(), entry.name.c_str());
                return nullptr;
            }

            layout.offsets.push_back(entry.offset);
        }

        // Try to find applier in already created applier
        for (MaterialApplier* applier : _M_appliers)
        {
            if (applier->_M_layout == layout)
                return applier;
        }

        // Creating new applier
        String applier_name = Strings::format("{}.Applier {}", name(), _M_appliers.size());
        Shader* shader      = Object::new_instance_named<Shader>(applier_name);
        shader->load(*_M_resources);

        MaterialApplier* applier = new MaterialApplier(shader, this);
        applier->_M_layout       = std::move(layout);
        _M_appliers.push_back(applier);
        return applier;
    }

    const Material& Material::apply_resources() const
    {
        for (auto& entry : _M_textures) entry.second.ptr()->bind(entry.first);
        return *this;
    }

    bool Material::archive_process(Archive* archive)
    {
        if (!ShaderResource::archive_process(archive))
        {
            return false;
        }

        // TODO

        return true;
    }

    Material& Material::add_texture(BindingIndex index, Texture* texture)
    {
        if (texture)
            _M_textures[index] = texture;
        else
            remove_texture(index);
        return *this;
    }

    Material& Material::remove_texture(BindingIndex index)
    {
        _M_textures.erase(index);
        return *this;
    }

    const Material::TexturesMap& Material::textures() const
    {
        return _M_textures;
    }

    Material::~Material()
    {
        for (MaterialApplier* applier : _M_appliers)
        {
            applier->_M_shader->mark_for_delete();
            delete applier;
        }

        _M_appliers.clear();
    }

    MaterialApplier::MaterialApplier(Shader* shader, Material* material, BindingIndex global_ubo_index)
        : _M_shader(shader), _M_material(material), _M_global_ubo_index(global_ubo_index)
    {}

    MaterialApplier::~MaterialApplier()
    {}

    MaterialApplier& MaterialApplier::apply()
    {
        _M_shader->use();
        _M_material->apply_resources();
        return *this;
    }

    Shader* MaterialApplier::shader() const
    {
        return _M_shader;
    }

    Material* MaterialApplier::material() const
    {
        return _M_material;
    }


    register_class(Engine::Material);
}// namespace Engine
