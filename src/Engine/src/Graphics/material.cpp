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
#include <Graphics/texture.hpp>

namespace Engine
{
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

    implement_engine_class_default_init(MaterialInterface);

    implement_engine_class(Material, Class::IsAsset);
    implement_initialize_class(Material)
    {
        Class* self = static_class_instance();

        self->add_properties(new ObjectProperty("Pipeline", "Pipeline settings for this material", &Material::pipeline));
    }

    implement_engine_class(MaterialInstance, Class::IsAsset);
    implement_default_initialize_class(MaterialInstance);


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
        return pipeline->archive_process(archive);
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
                new_param_allocator(BoolMaterialParameter), new_param_allocator(FloatMaterialParameter),
                new_param_allocator(IntMaterialParameter),  new_param_allocator(Vec2MaterialParameter),
                new_param_allocator(Vec3MaterialParameter), new_param_allocator(Vec4MaterialParameter),
                new_param_allocator(Mat3MaterialParameter), new_param_allocator(Mat4MaterialParameter),
        };

        return allocators;
    }


    MaterialParameter* Material::create_parameter(const Name& name, MaterialParameter::Type type)
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

        param = params_allocators()[static_cast<EnumerateType>(type)](name);
        _M_material_parameters[name] = param;
        return param;
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

    bool Material::apply()
    {
        return apply(this);
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

        return true;
    }

    class Material* Material::material()
    {
        return this;
    }

    Material::~Material()
    {
        delete pipeline;
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
        return parent_material.ptr();
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
