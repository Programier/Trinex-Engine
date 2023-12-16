#include <Core/class.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Engine/ActorComponents/mesh_component.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture.hpp>


namespace Engine
{
    implement_class(MaterialInterface, "Engine", 0);
    implement_class(Material, "Engine", 0);
    implement_class(MaterialInstance, "Engine", 0);
    implement_default_initialize_class(MaterialInterface);
    implement_default_initialize_class(Material);
    implement_default_initialize_class(MaterialInstance);


    MaterialParameter& MaterialParameter::name(Name new_name)
    {
        if (_M_material)
        {
            Name current_name                     = _M_name;
            Type parameter_type                   = type();
            MaterialParameter* material_parameter = _M_material->find_parameter(current_name, parameter_type);
            if (material_parameter)
            {
                material_parameter->_M_name = new_name;

                for (MaterialInstance* instance : _M_material->instances())
                {
                    MaterialParameter* parameter = instance->find_parameter(current_name, parameter_type);
                    if (parameter)
                    {
                        parameter->_M_name = new_name;
                    }
                }
            }
        }

        return *this;
    }

    bool MaterialParameter::is_valid() const
    {
        return true;
    }

    Material* MaterialParameter::material() const
    {
        return _M_material;
    }

    bool MaterialParameter::equal(MaterialParameter* param) const
    {
        return param->type() == type() && param->_M_name == _M_name && param->_M_material == _M_material;
    }

    Name MaterialParameter::name() const
    {
        return _M_name;
    }

    void MaterialParameter::copy_from(const MaterialParameter* from)
    {
        _M_name = from->_M_name;
    }

    MaterialParameter::~MaterialParameter()
    {}


    template<typename T>
    static MaterialParameter* static_find_parameter(Name name, const Vector<T*>& params)
    {
        for (MaterialParameter* param : params)
        {
            if (param->name() == name)
                return param;
        }

        return nullptr;
    }

    MaterialParameter* MaterialParameters::find_parameter(Name name, MaterialParameter::Type type) const
    {
        switch (type)
        {
            case MaterialParameter::Type::Texture:
                return static_find_parameter(name, textures);
            case MaterialParameter::Type::Sampler:
                return static_find_parameter(name, samplers);
            case MaterialParameter::Type::CombinedSampler:
                return static_find_parameter(name, combined_samplers);
        }

        return nullptr;
    }


    template<typename ParameterType, typename Lambda>
    static FORCE_INLINE void create_new_parameters(const Vector<ParameterType*>& in, Vector<ParameterType*>& out,
                                                   Lambda&& on_alloc)
    {
        for (ParameterType* in_parameter : in)
        {
            bool need_create = true;

            for (ParameterType* out_parameter : out)
            {
                if (out_parameter->equal(in_parameter))
                {
                    need_create = false;
                    break;
                }
            }

            if (need_create)
            {
                ParameterType* new_parameter = new ParameterType();
                new_parameter->copy_from(in_parameter);
                out.push_back(new_parameter);
            }
        }
    }

    void MaterialParameters::rebuild(Material* material)
    {
        auto predicate = [material](MaterialParameter* parameter) -> bool {
            return parameter->material() != material ||
                   parameter->material()->find_parameter(parameter->name(), parameter->type()) == nullptr;
        };

        std::erase_if(textures, predicate);
        std::erase_if(samplers, predicate);
        std::erase_if(combined_samplers, predicate);

        const MaterialParameters& material_parameters = material->parameters();
        auto on_alloc = [material](MaterialParameter* param) { param->_M_material = material; };

        create_new_parameters(material_parameters.textures, textures, on_alloc);
        create_new_parameters(material_parameters.samplers, samplers, on_alloc);
        create_new_parameters(material_parameters.combined_samplers, combined_samplers, on_alloc);
    }

    MaterialParameter* MaterialInterface::find_parameter(Name name, MaterialParameter::Type type) const
    {
        return parameters().find_parameter(name, type);
    }

    const Vector<VertexBufferStream>& Material::streams() const
    {
        return _M_streams;
    }

    /////////////////////// MATERIAL PARAMETERS ///////////////////////

    void MaterialBindingParameter::copy_from(const MaterialParameter* from)
    {
        Super::copy_from(from);
        location = static_cast<const MaterialBindingParameter*>(from)->location;
    }

    bool MaterialTextureParameter::is_valid() const
    {
        if (texture.ptr() == nullptr)
            return false;
        return Super::is_valid();
    }

    MaterialTextureParameter::Type MaterialTextureParameter::type() const
    {
        return MaterialTextureParameter::Type::Texture;
    }

    const MaterialTextureParameter& MaterialTextureParameter::set() const
    {
        texture->rhi_bind(location);
        return *this;
    }

    void MaterialTextureParameter::copy_from(const MaterialParameter* from)
    {
        Super::copy_from(from);
        texture = static_cast<const MaterialTextureParameter*>(from)->texture;
    }

    MaterialParameter::Type MaterialTextureParameter::static_type()
    {
        return MaterialParameter::Type::Texture;
    }

    bool MaterialSamplerParameter::is_valid() const
    {
        if (sampler.ptr() == nullptr)
            return false;
        return Super::is_valid();
    }

    MaterialSamplerParameter::Type MaterialSamplerParameter::type() const
    {
        return MaterialSamplerParameter::Type::Sampler;
    }

    const MaterialSamplerParameter& MaterialSamplerParameter::set() const
    {
        sampler->rhi_bind(location);
        return *this;
    }

    MaterialParameter::Type MaterialSamplerParameter::static_type()
    {
        return MaterialParameter::Type::Sampler;
    }

    void MaterialSamplerParameter::copy_from(const MaterialParameter* from)
    {
        Super::copy_from(from);
        sampler = static_cast<const MaterialSamplerParameter*>(from)->sampler;
    }

    bool MaterialCombinedSamplerParameter::is_valid() const
    {
        if (texture.ptr() == nullptr || sampler.ptr() == nullptr)
            return false;
        return Super::is_valid();
    }

    MaterialCombinedSamplerParameter::Type MaterialCombinedSamplerParameter::type() const
    {
        return MaterialCombinedSamplerParameter::Type::CombinedSampler;
    }

    const MaterialCombinedSamplerParameter& MaterialCombinedSamplerParameter::set() const
    {
        texture->bind_combined(sampler.ptr(), location);
        return *this;
    }

    MaterialParameter::Type MaterialCombinedSamplerParameter::static_type()
    {
        return MaterialParameter::Type::CombinedSampler;
    }

    void MaterialCombinedSamplerParameter::copy_from(const MaterialParameter* from)
    {
        Super::copy_from(from);
        sampler = static_cast<const MaterialCombinedSamplerParameter*>(from)->sampler;
        texture = static_cast<const MaterialCombinedSamplerParameter*>(from)->texture;
    }


    static FORCE_INLINE void static_apply_parameter(MaterialParameter* parameter, const MaterialInterface* owner,
                                                    const Material* material)
    {
        if (parameter->is_valid())
        {
            parameter->set();
        }
        else if (owner != material)
        {
            parameter = material->find_parameter(parameter->name(), parameter->type());
            if (parameter->is_valid())
            {
                parameter->set();
            }
        }
    }

    template<typename ParameterType>
    static FORCE_INLINE void static_apply_parameters(const Vector<ParameterType*>& parameters,
                                                     const MaterialInterface* owner, const Material* material)
    {
        for (ParameterType* parameter : parameters)
        {
            static_apply_parameter(parameter, owner, material);
        }
    }


    void MaterialParameters::apply(const class MaterialInterface* owner, const class Material* material) const
    {
        static_apply_parameters(textures, owner, material);
        static_apply_parameters(samplers, owner, material);
        static_apply_parameters(combined_samplers, owner, material);
    }

    MaterialParameters::~MaterialParameters()
    {
        for (auto* param : textures) delete param;
        for (auto* param : samplers) delete param;
        for (auto* param : combined_samplers) delete param;

        textures.clear();
        samplers.clear();
        combined_samplers.clear();
    }

    Material& Material::pipeline(Pipeline* pipeline)
    {
        _M_pipeline = pipeline;
        return *this;
    }

    const Set<class MaterialInstance*>& Material::instances() const
    {
        return _M_material_instances;
    }

    Pipeline* Material::pipeline()
    {
        return _M_pipeline.ptr();
    }

    MaterialParameter* Material::allocate_new_parameter(Name parameter_name, MaterialParameter::Type type,
                                                        MaterialParameter* (*allocator)())
    {
        if (find_parameter(parameter_name, type))
            return nullptr;

        MaterialParameter* parameter = allocator();
        parameter->_M_name           = parameter_name;


        switch (type)
        {
            case MaterialParameter::Type::CombinedSampler:
                _M_parameters.combined_samplers.push_back(static_cast<MaterialCombinedSamplerParameter*>(parameter));
                break;
            case MaterialParameter::Type::Texture:
                _M_parameters.textures.push_back(static_cast<MaterialTextureParameter*>(parameter));
                break;
            case MaterialParameter::Type::Sampler:
                _M_parameters.samplers.push_back(static_cast<MaterialSamplerParameter*>(parameter));
                break;
        }

        return parameter;
    }

    class MaterialInstance* Material::create_instance()
    {
        MaterialInstance* instance = Object::new_instance<MaterialInstance>(this);
        _M_material_instances.insert(instance);
        return instance;
    }

    Material& Material::preload()
    {
        return *this;
    }

    Material& Material::postload()
    {
        return *this;
    }

    Material& Material::push_stream(const VertexBufferStream& new_stream)
    {
        for (const VertexBufferStream& stream : _M_streams)
        {
            if (stream == new_stream)
            {
                return *this;
            }
        }

        _M_streams.push_back(new_stream);
        return *this;
    }

    Material& Material::remove_stream(Index index)
    {
        if (_M_streams.size() > index)
        {
            _M_streams.erase(_M_streams.begin() + index);
        }

        return *this;
    }

    Material& Material::remove_stream(const VertexBufferStream& stream)
    {
        for (Index i = 0, j = _M_streams.size(); i < j; ++i)
        {
            if (_M_streams[i] == stream)
            {
                return remove_stream(i);
            }
        }

        return *this;
    }


    bool Material::apply_internal(bool is_material) const
    {
        if (_M_pipeline == nullptr)
        {
            return false;
        }

        _M_pipeline->rhi_bind();

        if (is_material)
        {
            _M_parameters.apply(this, this);
        }
        return true;
    }

    bool Material::apply() const
    {
        return apply_internal(true);
    }

    Material* Material::material()
    {
        return this;
    }

    const Material* Material::material() const
    {
        return this;
    }

    const MaterialParameters& Material::parameters() const
    {
        return _M_parameters;
    }

    MaterialInstance::MaterialInstance(Material* material) : _M_material(material)
    {
        rebuild();
    }

    const Vector<VertexBufferStream>& MaterialInstance::streams() const
    {
        return _M_material->streams();
    }

    Material* MaterialInstance::material()
    {
        return const_cast<Material*>(_M_material.ptr());
    }

    const Material* MaterialInstance::material() const
    {
        return const_cast<Material*>(_M_material.ptr());
    }

    MaterialInstance& MaterialInstance::rebuild()
    {
        if (!_M_material)
            return *this;
        _M_parameters.rebuild(_M_material);
        return *this;
    }

    Pipeline* MaterialInstance::pipeline()
    {
        return _M_material->pipeline();
    }

    bool MaterialInstance::apply() const
    {
        if (!_M_material->apply_internal(false))
        {
            return false;
        }

        _M_parameters.apply(this, _M_material);
        return true;
    }

    const MaterialParameters& MaterialInstance::parameters() const
    {
        return _M_parameters;
    }
}// namespace Engine
