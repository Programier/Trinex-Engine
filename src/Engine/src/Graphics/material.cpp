#include <Core/archive.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/logger.hpp>
#include <Core/property.hpp>
#include <Core/render_thread.hpp>
#include <Core/string_functions.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_compiler.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine
{
    bool MaterialParameter::serialize_data(Archive& ar, void* data, size_t size)
    {
        if (ar.is_reading())
        {
            return ar.read_data(reinterpret_cast<byte*>(data), size);
        }
        else if (ar.is_saving())
        {
            ar.write_data(reinterpret_cast<const byte*>(data), size);
        }
        return false;
    }

    size_t MaterialParameter::size() const
    {
        return 0;
    }

    size_t MaterialParameter::offset() const
    {
        return no_offset;
    }

    MaterialParameter& MaterialParameter::offset(size_t)
    {
        return *this;
    }

    byte* MaterialParameter::data()
    {
        return nullptr;
    }

    const byte* MaterialParameter::data() const
    {
        return nullptr;
    }

    MaterialParameterType MaterialParameter::binding_object_type() const
    {
        return type();
    }

    MaterialParameter& MaterialParameter::apply(const Pipeline* pipeline, class SceneComponent* component)
    {
        if (static_cast<EnumerateType>(type()) <= static_cast<EnumerateType>(MaterialParameterType::Mat4))
        {
            size_t parameter_offset = offset();

            if (parameter_offset != no_offset)
            {
                engine_instance->rhi()->update_local_parameter(data(), size(), parameter_offset);
            }
        }

        return *this;
    }

    implement_engine_class_default_init(MaterialInterface);

    implement_engine_class(Material, Class::IsAsset);
    implement_initialize_class(Material)
    {
        Class* self = static_class_instance();
        self->add_properties(new ObjectProperty("Pipeline", "Pipeline settings for this material", &Material::pipeline,
                                                Name::none, Property::IsNotSerializable));
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
                MaterialParameterType type = param->type();
                Name name                  = _name;
                ar & name;
                ar & type;

                param->archive_process(ar);
            }
        }
        else if (ar.is_reading())
        {
            Name name;
            MaterialParameterType type;

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

    bool BindingMaterialParameter::archive_status(Archive& ar)
    {
        return static_cast<bool>(ar);
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

    SamplerMaterialParameter::SamplerMaterialParameter() : sampler(DefaultResources::default_sampler)
    {}

    MaterialParameterType SamplerMaterialParameter::type() const
    {
        return MaterialParameterType::Sampler;
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

    TextureMaterialParameterBase& TextureMaterialParameterBase::apply(const Pipeline* pipeline, SceneComponent*)
    {
        if (Texture* texture = texture_param())
        {
            texture->rhi_bind(location);
        }
        return *this;
    }

    CombinedImageSamplerMaterialParameterBase& CombinedImageSamplerMaterialParameterBase::apply(const Pipeline* pipeline,
                                                                                                SceneComponent* component)
    {
        auto texture = texture_param();
        auto sampler = sampler_param();

        if (texture && sampler)
        {
            texture->rhi_bind_combined(sampler, location);
        }
        return *this;
    }

    MaterialParameterType ModelMatrixMaterialParameter::type() const
    {
        return MaterialParameterType::ModelMatrix;
    }

    ModelMatrixMaterialParameter& ModelMatrixMaterialParameter::apply(const Pipeline* pipeline, SceneComponent* component)
    {
        size_t parameter_offset = offset();
        if (parameter_offset != no_offset)
        {
            Matrix4f model = component ? component->world_transform().matrix() : Matrix4f(1.f);
            engine_instance->rhi()->update_local_parameter(reinterpret_cast<const byte*>(&model), sizeof(model),
                                                           parameter_offset);
        }
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

        archive & compile_definitions;
        serialize_parameters(m_material_parameters, archive);
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
    declare_allocator(CombinedImageSampler2D);
    declare_allocator(ModelMatrix);


#define new_param_allocator(type)                                                                                                \
    case MaterialParameterType::type:                                                                                            \
        return type##_material_param_allocator;


    static MaterialParameter* (*find_param_allocator(MaterialParameterType type))()
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
            new_param_allocator(CombinedImageSampler2D);
            new_param_allocator(Texture2D);
            new_param_allocator(ModelMatrix);

            default:
                return nullptr;
        }
    }


    MaterialParameter* Material::create_parameter_internal(const Name& name, MaterialParameterType type)
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

    MaterialParameter* Material::create_parameter(const Name& name, MaterialParameterType type)
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

    const Material::ParametersMap& Material::parameters() const
    {
        return m_material_parameters;
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
            if (parameter && parameter->binding_object_type() == MaterialParameterType::Texture2D)
            {
                parameter->apply(pipeline, component);
            }
        }

        for (auto& sampler : shader->samplers)
        {
            MaterialParameter* parameter = head->find_parameter(sampler.name);
            if (parameter && parameter->binding_object_type() == MaterialParameterType::Sampler)
            {
                parameter->apply(pipeline, component);
            }
        }
    }

    bool Material::apply(MaterialInterface* head, SceneComponent* component)
    {
        trinex_check(is_in_render_thread(), "Material::apply method must be called in render thread!");
        pipeline->rhi_bind();

        for (auto& [name, material_parameter] : m_material_parameters)
        {
            MaterialParameter* parameter = head->find_parameter(name);
            if (parameter)
            {
                parameter->apply(pipeline, component);
            }
        }

        apply_shader_global_params(pipeline->vertex_shader(), head, component);
        apply_shader_global_params(pipeline->fragment_shader(), head, component);

        return true;
    }

    class Material* Material::material()
    {
        return this;
    }

    static bool submit_compiled_source(Material* material, const ShaderCompiler::ShaderSource& source, MessageList* errors)
    {
        bool status = false;

        bool has_valid_graphical_pipeline = source.has_valid_graphical_pipeline();
        bool has_valid_compute_pipiline   = source.has_valid_compute_pipeline();

        if (has_valid_graphical_pipeline || has_valid_compute_pipiline)
        {
            material->pipeline->remove_all_shaders();
        }

        if (source.has_valid_graphical_pipeline())
        {
            render_thread()->wait_all();


            Pipeline* pipeline   = material->pipeline;
            auto vertex_shader   = pipeline->vertex_shader(true);
            auto fragment_shader = pipeline->fragment_shader(true);

            vertex_shader->attributes.clear();
            vertex_shader->attributes.reserve(source.reflection.attributes.size());


            for (auto& attribute : source.reflection.attributes)
            {
                VertexShader::Attribute out_attribute;
                out_attribute.name           = attribute.name;
                out_attribute.format         = attribute.format;
                out_attribute.rate           = attribute.rate;
                out_attribute.semantic       = attribute.semantic;
                out_attribute.semantic_index = attribute.semantic_index;
                out_attribute.count          = attribute.count;

                vertex_shader->attributes.push_back(out_attribute);
            }

            vertex_shader->source_code   = source.vertex_code;
            fragment_shader->source_code = source.fragment_code;

            if (source.has_tessellation_control_shader())
            {
                pipeline->tessellation_control_shader(true)->source_code = source.tessellation_control_code;
            }
            else
            {
                pipeline->remove_tessellation_control_shader();
            }

            if (source.has_tessellation_shader())
            {
                pipeline->tessellation_shader(true)->source_code = source.tessellation_code;
            }
            else
            {
                pipeline->remove_tessellation_shader();
            }

            if (source.has_geometry_shader())
            {
                pipeline->geometry_shader(true)->source_code = source.geometry_code;
            }
            else
            {
                pipeline->remove_geometry_shader();
            }

            TreeSet<Name> names_to_remove;

            for (auto& entry : material->parameters())
            {
                names_to_remove.insert(entry.first);
            }

            for (auto& parameter : source.reflection.uniform_member_infos)
            {
                Name name = parameter.name;
                names_to_remove.erase(name);
                MaterialParameter* material_parameter = material->find_parameter(name);

                if (material_parameter && material_parameter->type() != parameter.type)
                {
                    material->remove_parameter(name);
                    material_parameter = nullptr;
                }

                if (!material_parameter)
                {
                    if (!(material_parameter = material->create_parameter(name, parameter.type)))
                    {
                        error_log("Material", "Failed to create material parameter '%s'", name.c_str());
                        continue;
                    }
                }

                material_parameter->offset(parameter.offset);
            }

            for (auto& name : names_to_remove)
            {
                material->remove_parameter(name);
            }

            material->pipeline->global_parameters = source.reflection.global_parameters_info;
            material->pipeline->local_parameters  = source.reflection.local_parameters_info;
            material->apply_changes();
            status = true;
        }

        return status;
    }

    bool Material::compile(ShaderCompiler::Compiler* compiler, MessageList* errors)
    {
        static MessageList dummy_message_list;

        if (errors == nullptr)
        {
            errors = &dummy_message_list;
        }

        errors->clear();

        bool need_delete_compiler = compiler == nullptr;

        if (need_delete_compiler)
        {
            Class* compiler_class = Class::static_find(Strings::format("Engine::ShaderCompiler::{}_Compiler", engine_config.api));
            if (!compiler_class)
            {
                error_log("Material", "Failed to find shader compiler!");
                return false;
            }

            auto compiler_instance = compiler_class->create_object();

            if (compiler_instance)
            {
                compiler = Object::instance_cast<ShaderCompiler::Compiler>(compiler_instance);
            }

            if (!compiler)
            {
                error_log("Material", "Failed to create material compiler!");

                if (compiler_instance)
                {
                    delete compiler_instance;
                }
            }
        }

        ShaderCompiler::ShaderSource source;
        auto status = compiler->compile(this, source, *errors);

        if (status)
        {
            status = submit_compiled_source(this, source, errors);
        }

        if (need_delete_compiler)
        {
            delete compiler;
        }

        return status;
    }

    Material& Material::apply_changes()
    {
        return postload();
    }

    Material::~Material()
    {
        delete pipeline;
    }

    MaterialParameter* MaterialInstance::create_parameter_internal(const Name& name, MaterialParameterType type)
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
