#include <Core/archive.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Core/property.hpp>
#include <Core/render_thread.hpp>
#include <Core/string_functions.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/scene_render_targets.hpp>
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

    MaterialParameter::Type MaterialParameter::binding_object_type() const
    {
        return type();
    }

    MaterialParameter& MaterialParameter::apply(const Pipeline* pipeline, class SceneComponent* component)
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


    void BindingMaterialParameter::bind_texture(class Engine::Texture2D* texture)
    {}

    void BindingMaterialParameter::bind_sampler(class Engine::Sampler* sampler)
    {
        if (sampler)
        {
            sampler->rhi_bind(location);
        }
    }

    void BindingMaterialParameter::bind_combined(class Engine::Sampler* sampler, class Engine::Texture2D* texture)
    {
        if (texture && sampler)
        {
            texture->rhi_bind_combined(sampler, location);
        }
    }


    bool BindingMaterialParameter::archive_process(Archive& ar)
    {
        ar & location;
        return ar;
    }

    class Texture* BindingMaterialParameter::texture_param() const
    {
        return nullptr;
    }

    class Sampler* BindingMaterialParameter::sampler_param() const
    {
        return nullptr;
    }

    BindingMaterialParameter& BindingMaterialParameter::texture_param(class Texture* texture)
    {
        return *this;
    }

    BindingMaterialParameter& BindingMaterialParameter::sampler_param(class Sampler* sampler)
    {
        return *this;
    }


    MaterialParameter::Type SamplerMaterialParameter::type() const
    {
        return MaterialParameter::Type::Sampler;
    }


    MaterialParameter& SamplerMaterialParameter::apply(const Pipeline* pipeline, SceneComponent* component)
    {
        bind_sampler(sampler);
        return *this;
    }

    bool SamplerMaterialParameter::archive_process(Archive& ar)
    {
        if (!BindingMaterialParameter::archive_process(ar))
            return false;

        sampler.archive_process(ar, true);
        return ar;
    }

    class Sampler* SamplerMaterialParameter::sampler_param() const
    {
        return sampler.ptr();
    }

    SamplerMaterialParameter& SamplerMaterialParameter::sampler_param(class Sampler* new_sampler)
    {
        sampler = new_sampler;
        return *this;
    }

    MaterialParameter::Type CombinedTexture2DMaterialParameter::type() const
    {
        return MaterialParameter::Type::CombinedTexture2D;
    }

    MaterialParameter& CombinedTexture2DMaterialParameter::apply(const Pipeline* pipeline, SceneComponent* component)
    {
        bind_combined(sampler, texture);
        return *this;
    }

    bool CombinedTexture2DMaterialParameter::archive_process(Archive& ar)
    {
        if (!BindingMaterialParameter::archive_process(ar))
            return false;

        texture.archive_process(ar, true);
        sampler.archive_process(ar, true);
        return ar;
    }

    Texture* CombinedTexture2DMaterialParameter::texture_param() const
    {
        return texture.ptr();
    }

    Sampler* CombinedTexture2DMaterialParameter::sampler_param() const
    {
        return sampler.ptr();
    }

    CombinedTexture2DMaterialParameter& CombinedTexture2DMaterialParameter::texture_param(class Texture* new_texture)
    {
        if (new_texture == nullptr)
        {
            texture = nullptr;
        }
        else if (class Texture2D* new_texture_2d = Object::instance_cast<class Texture2D>(new_texture))
        {
            texture = new_texture_2d;
        }
        return *this;
    }

    CombinedTexture2DMaterialParameter& CombinedTexture2DMaterialParameter::sampler_param(class Sampler* new_sampler)
    {
        sampler = new_sampler;
        return *this;
    }


    MaterialParameter::Type Texture2DMaterialParameter::type() const
    {
        return MaterialParameter::Type::Texture2D;
    }

    MaterialParameter& Texture2DMaterialParameter::apply(const Pipeline* pipeline, SceneComponent* component)
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

    class Texture* Texture2DMaterialParameter::texture_param() const
    {
        return texture.ptr();
    }

    Texture2DMaterialParameter& Texture2DMaterialParameter::texture_param(class Texture* new_texture)
    {
        if (new_texture == nullptr)
        {
            texture = nullptr;
        }
        else if (class Texture2D* new_texture_2d = Object::instance_cast<class Texture2D>(new_texture))
        {
            texture = new_texture_2d;
        }

        return *this;
    }

    ModelMatrixMaterialParameter::Type ModelMatrixMaterialParameter::type() const
    {
        return Type::ModelMatrix;
    }

    ModelMatrixMaterialParameter& ModelMatrixMaterialParameter::apply(const Pipeline* pipeline, SceneComponent* component)
    {
        size_t offset = pipeline->local_parameters.offset_of(name);
        if (offset != LocalMaterialParametersInfo::no_offset)
        {
            Matrix4f model = component ? component->world_transform().matrix() : Matrix4f(1.f);
            engine_instance->rhi()->update_local_parameter(reinterpret_cast<const byte*>(&model), sizeof(model), offset);
        }
        return *this;
    }

    BaseColorTextureMaterialParameter::Type BaseColorTextureMaterialParameter::type() const
    {
        return Type::BaseColorTexture;
    }

    MaterialParameter::Type BaseColorTextureMaterialParameter::binding_object_type() const
    {
        return Type::CombinedTexture2D;
    }

    BaseColorTextureMaterialParameter& BaseColorTextureMaterialParameter::apply(const Pipeline* pipeline,
                                                                                SceneComponent* component)
    {
        bind_combined(sampler, GBuffer::instance()->current_frame()->base_color());
        return *this;
    }

    PositionTextureMaterialParameter::Type PositionTextureMaterialParameter::type() const
    {
        return Type::PositionTexture;
    }

    MaterialParameter::Type PositionTextureMaterialParameter::binding_object_type() const
    {
        return Type::CombinedTexture2D;
    }

    PositionTextureMaterialParameter& PositionTextureMaterialParameter::apply(const Pipeline* pipeline, SceneComponent* component)
    {
        bind_combined(sampler, GBuffer::instance()->current_frame()->position());
        return *this;
    }

    NormalTextureMaterialParameter::Type NormalTextureMaterialParameter::type() const
    {
        return Type::NormalTexture;
    }

    MaterialParameter::Type NormalTextureMaterialParameter::binding_object_type() const
    {
        return Type::CombinedTexture2D;
    }


    NormalTextureMaterialParameter& NormalTextureMaterialParameter::apply(const Pipeline* pipeline, SceneComponent* component)
    {
        bind_combined(sampler, GBuffer::instance()->current_frame()->normal());
        return *this;
    }

    EmissiveTextureMaterialParameter::Type EmissiveTextureMaterialParameter::type() const
    {
        return Type::EmissiveTexture;
    }

    MaterialParameter::Type EmissiveTextureMaterialParameter::binding_object_type() const
    {
        return Type::CombinedTexture2D;
    }

    EmissiveTextureMaterialParameter& EmissiveTextureMaterialParameter::apply(const Pipeline* pipeline, SceneComponent* component)
    {
        bind_combined(sampler, GBuffer::instance()->current_frame()->emissive());
        return *this;
    }

    DataBufferTextureMaterialParameter::Type DataBufferTextureMaterialParameter::type() const
    {
        return Type::DataBufferTexture;
    }

    MaterialParameter::Type DataBufferTextureMaterialParameter::binding_object_type() const
    {
        return Type::CombinedTexture2D;
    }

    DataBufferTextureMaterialParameter& DataBufferTextureMaterialParameter::apply(const Pipeline* pipeline,
                                                                                  SceneComponent* component)
    {
        bind_combined(sampler, GBuffer::instance()->current_frame()->data_buffer());
        return *this;
    }

    SceneOutputTextureMaterialParameter::Type SceneOutputTextureMaterialParameter::type() const
    {
        return Type::SceneOutputTexture;
    }

    MaterialParameter::Type SceneOutputTextureMaterialParameter::binding_object_type() const
    {
        return Type::CombinedTexture2D;
    }

    SceneOutputTextureMaterialParameter& SceneOutputTextureMaterialParameter::apply(const Pipeline* pipeline,
                                                                                    SceneComponent* component)
    {
        bind_combined(sampler, SceneColorOutput::instance()->current_frame()->texture());
        return *this;
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

    bool MaterialInterface::apply(SceneComponent* component)
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

        return serialize_parameters(m_material_parameters, archive);
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
        auto it = m_material_parameters.find(name);

        if (it != m_material_parameters.end())
        {
            return it->second;
        }

        return Super::find_parameter(name);
    }


#define declare_allocator(name)                                                                                                  \
    static MaterialParameter* name##_material_param_allocator()                                                                  \
    {                                                                                                                            \
        return new name##MaterialParameter();                                                                                    \
    }


    declare_allocator(Bool);
    declare_allocator(Int);
    declare_allocator(UInt);
    declare_allocator(Float);
    declare_allocator(BVec2);
    declare_allocator(BVec3);
    declare_allocator(BVec4);
    declare_allocator(IVec2);
    declare_allocator(IVec3);
    declare_allocator(IVec4);
    declare_allocator(UVec2);
    declare_allocator(UVec3);
    declare_allocator(UVec4);
    declare_allocator(Vec2);
    declare_allocator(Vec3);
    declare_allocator(Vec4);
    declare_allocator(Mat3);
    declare_allocator(Mat4);
    declare_allocator(Sampler);
    declare_allocator(Texture2D);
    declare_allocator(CombinedTexture2D);
    declare_allocator(ModelMatrix);
    declare_allocator(BaseColorTexture);
    declare_allocator(PositionTexture);
    declare_allocator(NormalTexture);
    declare_allocator(EmissiveTexture);
    declare_allocator(DataBufferTexture);
    declare_allocator(SceneOutputTexture);


#define new_param_allocator(type)                                                                                                \
    case MaterialParameter::Type::type:                                                                                          \
        return type##_material_param_allocator;


    static MaterialParameter* (*find_param_allocator(MaterialParameter::Type type))()
    {
        switch (type)
        {
            new_param_allocator(Bool);
            new_param_allocator(Int);
            new_param_allocator(UInt);
            new_param_allocator(Float);
            new_param_allocator(BVec2);
            new_param_allocator(BVec3);
            new_param_allocator(BVec4);
            new_param_allocator(IVec2);
            new_param_allocator(IVec3);
            new_param_allocator(IVec4);
            new_param_allocator(UVec2);
            new_param_allocator(UVec3);
            new_param_allocator(UVec4);
            new_param_allocator(Vec2);
            new_param_allocator(Vec3);
            new_param_allocator(Vec4);
            new_param_allocator(Mat3);
            new_param_allocator(Mat4);
            new_param_allocator(Sampler);
            new_param_allocator(Texture2D);
            new_param_allocator(CombinedTexture2D);
            new_param_allocator(ModelMatrix);
            new_param_allocator(BaseColorTexture);
            new_param_allocator(PositionTexture);
            new_param_allocator(NormalTexture);
            new_param_allocator(EmissiveTexture);
            new_param_allocator(DataBufferTexture);
            new_param_allocator(SceneOutputTexture);

            default:
                return nullptr;
        }
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

        auto allocator = find_param_allocator(type);

        if (allocator)
        {
            param                       = allocator();
            param->name                 = name;
            m_material_parameters[name] = param;
            return param;
        }

        return nullptr;
    }

    MaterialParameter* Material::create_parameter(const Name& name, MaterialParameter::Type type)
    {
        return create_parameter_internal(name, type);
    }

    Material& Material::remove_parameter(const Name& name)
    {
        auto it = m_material_parameters.find(name);

        if (it != m_material_parameters.end())
        {
            delete it->second;
            m_material_parameters.erase(it);
        }
        return *this;
    }

    Material& Material::clear_parameters()
    {
        for (auto& [name, param] : m_material_parameters)
        {
            delete param;
        }

        m_material_parameters.clear();
        return *this;
    }

    bool Material::apply(SceneComponent* component)
    {
        return apply(this, component);
    }


    void Material::apply_shader_global_params(class Shader* shader, MaterialInterface* head, SceneComponent* component)
    {
        for (auto& texture : shader->textures)
        {
            MaterialParameter* parameter = head->find_parameter(texture.name);
            if (parameter && parameter->binding_object_type() == MaterialParameter::Type::Texture2D)
            {
                parameter->apply(pipeline, component);
            }
        }

        for (auto& sampler : shader->samplers)
        {
            MaterialParameter* parameter = head->find_parameter(sampler.name);
            if (parameter && parameter->binding_object_type() == MaterialParameter::Type::Sampler)
            {
                parameter->apply(pipeline, component);
            }
        }

        for (auto& texture : shader->combined_samplers)
        {
            MaterialParameter* parameter = head->find_parameter(texture.name);
            if (parameter && parameter->binding_object_type() == MaterialParameter::Type::CombinedTexture2D)
            {
                parameter->apply(pipeline, component);
            }
        }
    }

    bool Material::apply(MaterialInterface* head, SceneComponent* component)
    {
        trinex_check(is_in_render_thread(), "Material::apply method must be called in render thread!");
        pipeline->rhi_bind();

        for (auto& [name, offset] : pipeline->local_parameters.offset_map())
        {
            MaterialParameter* parameter = head->find_parameter(name);
            if (parameter)
            {
                parameter->apply(pipeline, component);
            }
        }

        apply_shader_global_params(pipeline->vertex_shader, head, component);
        apply_shader_global_params(pipeline->fragment_shader, head, component);

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

        auto allocator = find_param_allocator(type);
        if (allocator)
        {
            param                       = allocator();
            m_material_parameters[name] = param;
            return param;
        }
        return nullptr;
    }

    bool MaterialInstance::archive_process(Archive& archive)
    {
        if (!Super::archive_process(archive))
            return false;
        return serialize_parameters(m_material_parameters, archive);
    }

    MaterialParameter* MaterialInstance::find_parameter(const Name& name) const
    {
        auto it = m_material_parameters.find(name);

        if (it != m_material_parameters.end())
        {
            return it->second;
        }

        return Super::find_parameter(name);
    }

    MaterialInterface* MaterialInstance::parent() const
    {
        return parent_material;
    }

    bool MaterialInstance::apply(SceneComponent* component)
    {
        Material* mat = material();
        if (!mat)
        {
            return false;
        }

        return mat->apply(this, component);
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
