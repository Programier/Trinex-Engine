#include <Core/archive.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Core/property.hpp>
#include <Core/render_thread.hpp>
#include <Core/string_functions.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine
{
    size_t MaterialParameter::size() const
    {
        return 0;
    }

    byte* MaterialParameter::data()
    {
        return nullptr;
    }

    const byte* MaterialParameter::data() const
    {
        return nullptr;
    }

    MaterialParameter& MaterialParameter::apply(const Pipeline* pipeline)
    {
        if (static_cast<EnumerateType>(type()) <= static_cast<EnumerateType>(Type::Mat4))
        {
            size_t offset = pipeline->local_parameters.offset_of(name);
            if (offset != LocalMaterialParametersInfo::no_offset)
            {
                engine_instance->rhi()->update_local_parameter(data(), size(), offset);
            }
        }

        return *this;
    }

    bool MaterialParameter::archive_process(Archive& ar)
    {
        if (static_cast<EnumerateType>(type()) <= static_cast<EnumerateType>(Type::Mat4))
        {
            if (ar.is_reading())
            {
                ar.read_data(data(), size());
            }
            else if (ar.is_saving())
            {
                ar.write_data(data(), size());
            }
        }
        return ar;
    }

    implement_engine_class_default_init(MaterialInterface);

    implement_engine_class(Material, Class::IsAsset);
    implement_initialize_class(Material)
    {
        Class* self = static_class_instance();

        self->add_properties(new ObjectProperty("Pipeline", "Pipeline settings for this material", &Material::pipeline));
    }

    implement_engine_class(MaterialInstance, Class::IsAsset);
    implement_initialize_class(MaterialInstance)
    {
        Class* self = MaterialInstance::static_class_instance();
        self->add_property(new ObjectReferenceProperty("Parent Material", "Parent Material of this instance",
                                                       &MaterialInstance::parent_material));
    }


    bool MaterialInterface::serialize_parameters(Map<Name, MaterialParameter*, Name::HashFunction>& map, Archive& ar)
    {
        size_t count = map.size();
        ar & count;
        if (ar.is_saving())
        {
            for (auto& [_name, param] : map)
            {
                MaterialParameter::Type type = param->type();
                Name name                    = _name;
                ar & name;
                ar & type;

                param->archive_process(ar);
            }
        }
        else if (ar.is_reading())
        {
            Name name;
            MaterialParameter::Type type;

            while (count > 0)
            {
                ar & name;
                ar & type;

                MaterialParameter* param = create_parameter_internal(name, type);

                if (param)
                {
                    param->archive_process(ar);
                }
                else
                {
                    return false;
                }

                --count;
            }
        }
        return ar;
    }


    bool BindingMaterialParameter::archive_process(Archive& ar)
    {
        ar & location;
        return ar;
    }

    MaterialParameter::Type SamplerMaterialParameter::type() const
    {
        return MaterialParameter::Type::Sampler;
    }

    MaterialParameter& SamplerMaterialParameter::apply(const Pipeline* pipeline)
    {
        if (sampler)
        {
            sampler->rhi_bind(location);
        }
        return *this;
    }

    bool SamplerMaterialParameter::archive_process(Archive& ar)
    {
        if (!BindingMaterialParameter::archive_process(ar))
            return false;

        sampler.archive_process(ar, true);
        return ar;
    }

    MaterialParameter::Type CombinedSampler2DMaterialParameter::type() const
    {
        return MaterialParameter::Type::CombinedSampler2D;
    }

    MaterialParameter& CombinedSampler2DMaterialParameter::apply(const Pipeline* pipeline)
    {
        if (texture && sampler)
        {
            texture->rhi_bind_combined(sampler.ptr(), location);
        }
        return *this;
    }

    bool CombinedSampler2DMaterialParameter::archive_process(Archive& ar)
    {
        if (!BindingMaterialParameter::archive_process(ar))
            return false;

        texture.archive_process(ar, true);
        sampler.archive_process(ar, true);
        return ar;
    }

    MaterialParameter::Type Texture2DMaterialParameter::type() const
    {
        return MaterialParameter::Type::Texture2D;
    }

    MaterialParameter& Texture2DMaterialParameter::apply(const Pipeline* pipeline)
    {
        if (texture)
        {
            texture->rhi_bind(location);
        }
        return *this;
    }

    bool Texture2DMaterialParameter::archive_process(Archive& ar)
    {
        if (!BindingMaterialParameter::archive_process(ar))
            return false;

        texture.archive_process(ar, true);
        return ar;
    }

    MaterialParameter* MaterialInterface::find_parameter(const Name& name) const
    {
        return nullptr;
    }

    MaterialInterface* MaterialInterface::parent() const
    {
        return nullptr;
    }

    class Material* MaterialInterface::material()
    {
        return nullptr;
    }

    bool MaterialInterface::apply()
    {
        return false;
    }

    Material::Material()
    {
        pipeline = Object::new_instance_named<Pipeline>("Pipeline");
        pipeline->flags(Object::IsAvailableForGC, false);
        pipeline->owner(this);
    }

    bool Material::archive_process(Archive& archive)
    {
        if (!Super::archive_process(archive))
            return false;

        return serialize_parameters(_M_material_parameters, archive);
    }

    Material& Material::preload()
    {
        pipeline->preload();
        return *this;
    }

    Material& Material::postload()
    {
        pipeline->postload();
        return *this;
    }

    MaterialParameter* Material::find_parameter(const Name& name) const
    {
        auto it = _M_material_parameters.find(name);

        if (it != _M_material_parameters.end())
        {
            return it->second;
        }

        return Super::find_parameter(name);
    }


#define new_param_allocator(type)                                                                                                \
    [](const Name& name) -> MaterialParameter* {                                                                                 \
        type* param = new type();                                                                                                \
        param->name = name;                                                                                                      \
        return param;                                                                                                            \
    }

    static const Array<MaterialParameter* (*) (const Name& name), MaterialParameter::Type::__COUNT__>& params_allocators()
    {
        static const Array<MaterialParameter* (*) (const Name& name), MaterialParameter::Type::__COUNT__> allocators = {
                new_param_allocator(BoolMaterialParameter),
                new_param_allocator(FloatMaterialParameter),
                new_param_allocator(IntMaterialParameter),
                new_param_allocator(Vec2MaterialParameter),
                new_param_allocator(Vec3MaterialParameter),
                new_param_allocator(Vec4MaterialParameter),
                new_param_allocator(Mat3MaterialParameter),
                new_param_allocator(Mat4MaterialParameter),
                new_param_allocator(SamplerMaterialParameter),
                new_param_allocator(Texture2DMaterialParameter),
                new_param_allocator(CombinedSampler2DMaterialParameter),
        };

        return allocators;
    }


    MaterialParameter* Material::create_parameter_internal(const Name& name, MaterialParameter::Type type)
    {
        MaterialParameter* param = find_parameter(name);
        if (param)
        {
            if (param->type() != type)
            {
                error_log("Material", "Failed to create new material parameter with type [%d]. Parameter with same name, but "
                                      "different type already exist!");
                return nullptr;
            }

            return param;
        }

        param                        = params_allocators()[static_cast<EnumerateType>(type)](name);
        _M_material_parameters[name] = param;
        return param;
    }

    MaterialParameter* Material::create_parameter(const Name& name, MaterialParameter::Type type)
    {
        return create_parameter_internal(name, type);
    }

    Material& Material::remove_parameter(const Name& name)
    {
        auto it = _M_material_parameters.find(name);

        if (it != _M_material_parameters.end())
        {
            delete it->second;
            _M_material_parameters.erase(it);
        }
        return *this;
    }

    Material& Material::clear_parameters()
    {
        for (auto& [name, param] : _M_material_parameters)
        {
            delete param;
        }

        _M_material_parameters.clear();
        return *this;
    }

    bool Material::apply()
    {
        return apply(this);
    }


    void Material::apply_shader_global_params(class Shader* shader, MaterialInterface* head)
    {
        for (auto& texture : shader->textures)
        {
            MaterialParameter* parameter = head->find_parameter(texture.name);
            if (parameter && parameter->type() == MaterialParameter::Type::Texture2D)
            {
                parameter->apply(pipeline);
            }
        }

        for (auto& sampler : shader->samplers)
        {
            MaterialParameter* parameter = head->find_parameter(sampler.name);
            if (parameter && parameter->type() == MaterialParameter::Type::Sampler)
            {
                parameter->apply(pipeline);
            }
        }

        for (auto& texture : shader->combined_samplers)
        {
            MaterialParameter* parameter = head->find_parameter(texture.name);
            if (parameter && parameter->type() == MaterialParameter::Type::CombinedSampler2D)
            {
                parameter->apply(pipeline);
            }
        }
    }

    bool Material::apply(MaterialInterface* head)
    {
        trinex_check(is_in_render_thread(), "Material::apply method must be called in render thread!");
        pipeline->rhi_bind();

        for (auto& [name, offset] : pipeline->local_parameters.offset_map())
        {
            MaterialParameter* parameter = head->find_parameter(name);
            if (parameter)
            {
                parameter->apply(pipeline);
            }
        }

        apply_shader_global_params(pipeline->vertex_shader, head);
        apply_shader_global_params(pipeline->fragment_shader, head);

        return true;
    }

    class Material* Material::material()
    {
        return this;
    }

    Material& Material::apply_changes()
    {
        return postload();
    }

    Material::~Material()
    {
        delete pipeline;
    }

    MaterialParameter* MaterialInstance::create_parameter_internal(const Name& name, MaterialParameter::Type type)
    {
        MaterialParameter* param = find_parameter(name);
        if (param)
        {
            if (param->type() != type)
            {
                error_log("MaterialInstance",
                          "Failed to create new material parameter with type [%d]. Parameter with same name, but "
                          "different type already exist!");
                return nullptr;
            }

            return param;
        }

        param                        = params_allocators()[static_cast<EnumerateType>(type)](name);
        _M_material_parameters[name] = param;
        return param;
    }

    bool MaterialInstance::archive_process(Archive& archive)
    {
        if (!Super::archive_process(archive))
            return false;
        return serialize_parameters(_M_material_parameters, archive);
    }

    MaterialParameter* MaterialInstance::find_parameter(const Name& name) const
    {
        auto it = _M_material_parameters.find(name);

        if (it != _M_material_parameters.end())
        {
            return it->second;
        }

        return Super::find_parameter(name);
    }

    MaterialInterface* MaterialInstance::parent() const
    {
        return parent_material;
    }

    bool MaterialInstance::apply()
    {
        Material* mat = material();
        if (!mat)
        {
            return false;
        }

        return mat->apply(this);
    }

    class Material* MaterialInstance::material()
    {
        if (parent_material)
        {
            return parent_material->material();
        }
        return nullptr;
    }
}// namespace Engine
