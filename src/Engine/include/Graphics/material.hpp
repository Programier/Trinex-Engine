#pragma once
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Core/structures.hpp>
#include <Graphics/texture.hpp>

namespace Engine
{
    class Pipeline;

    struct ENGINE_EXPORT VertexBufferStream {
        BindingIndex stream;
        VertexBufferSemantic semantic;
        byte index = 0;

        FORCE_INLINE bool operator==(const VertexBufferStream& other) const
        {
            return other.stream == stream && other.semantic == semantic && other.index == index;
        }

        FORCE_INLINE bool operator!=(const VertexBufferStream& other) const
        {
            return !((*this) == other);
        }
    };

    /////////////////// MATERIAL PARAMETERS ///////////////////

    class ENGINE_EXPORT MaterialParameter
    {
    public:
        enum class Type
        {
            Texture,
            Sampler,
            CombinedSampler,
        };

    private:
        Name _M_name;
        class Material* _M_material = nullptr;


    public:
        MaterialParameter& name(Name new_name);
        Name name() const;
        Material* material() const;

        virtual bool equal(MaterialParameter* param) const;

        virtual Type type() const                    = 0;
        virtual const MaterialParameter& set() const = 0;
        virtual void copy_from(const MaterialParameter* from);
        virtual bool is_valid() const;

        friend class Material;
        friend class MaterialInstance;
        friend struct MaterialParameters;

        virtual ~MaterialParameter();
    };

    class ENGINE_EXPORT MaterialBindingParameter : public MaterialParameter
    {
    public:
        using Super = MaterialParameter;

        BindLocation location;

        void copy_from(const MaterialParameter* from) override;
    };

    class ENGINE_EXPORT MaterialTextureParameter : public MaterialBindingParameter
    {
    public:
        using Super = MaterialBindingParameter;

        Pointer<Texture> texture;

        Type type() const override;
        const MaterialTextureParameter& set() const override;
        void copy_from(const MaterialParameter* from) override;
        bool is_valid() const override;

        static MaterialParameter::Type static_type();
    };

    class ENGINE_EXPORT MaterialSamplerParameter : public MaterialBindingParameter
    {
    public:
        using Super = MaterialBindingParameter;

        Pointer<Sampler> sampler;

        Type type() const override;
        const MaterialSamplerParameter& set() const override;
        void copy_from(const MaterialParameter* from) override;
        bool is_valid() const override;

        static MaterialParameter::Type static_type();
    };

    class ENGINE_EXPORT MaterialCombinedSamplerParameter : public MaterialBindingParameter
    {
    public:
        using Super = MaterialBindingParameter;

        Pointer<Texture> texture;
        Pointer<Sampler> sampler;

        Type type() const override;
        const MaterialCombinedSamplerParameter& set() const override;
        void copy_from(const MaterialParameter* from) override;
        bool is_valid() const override;

        static MaterialParameter::Type static_type();
    };


    struct MaterialParameters {
        Vector<MaterialTextureParameter*> textures;
        Vector<MaterialSamplerParameter*> samplers;
        Vector<MaterialCombinedSamplerParameter*> combined_samplers;


        MaterialParameter* find_parameter(Name name, MaterialParameter::Type type) const;

    private:
        void rebuild(Material* material);
        void apply(const class MaterialInterface* owner, const class Material* material) const;

        ~MaterialParameters();

    public:
        friend class MaterialInstance;
        friend class Material;
    };


    class ENGINE_EXPORT MaterialInterface : public Object
    {
        declare_class(MaterialInterface, Object);

    public:
        virtual const Vector<VertexBufferStream>& streams() const = 0;
        virtual Pipeline* pipeline()                              = 0;
        virtual bool apply() const                                = 0;

        virtual const MaterialParameters& parameters() const = 0;

        MaterialParameter* find_parameter(Name name, MaterialParameter::Type type) const;

        virtual Material* material()             = 0;
        virtual const Material* material() const = 0;
    };

    class ENGINE_EXPORT Material : public MaterialInterface
    {
        declare_class(Material, MaterialInterface);

    private:
        Pointer<Pipeline> _M_pipeline;
        Vector<VertexBufferStream> _M_streams;
        Set<class MaterialInstance*> _M_material_instances;
        MaterialParameters _M_parameters;

        MaterialParameter* allocate_new_parameter(Name parameter_name, MaterialParameter::Type type,
                                                  MaterialParameter* (*allocator)());

    public:
        const Vector<VertexBufferStream>& streams() const override;
        Pipeline* pipeline() override;
        Material& pipeline(Pipeline* pipeline);

        class MaterialInstance* create_instance();
        const Set<class MaterialInstance*>& instances() const;

        Material& preload() override;
        Material& postload() override;

        Material& push_stream(const VertexBufferStream& stream);
        Material& remove_stream(Index index);
        Material& remove_stream(const VertexBufferStream& stream);

        const MaterialParameters& parameters() const override;
        bool apply() const override;

        Material* material() override;
        const Material* material() const override;


        template<typename MaterialParameterType>
        FORCE_INLINE std::enable_if<std::is_base_of_v<MaterialParameter, MaterialParameterType>,
                                    MaterialParameterType*>::type
        create_parameter(Name parameter_name)
        {
            return static_cast<MaterialParameterType*>(
                    allocate_new_parameter(parameter_name, MaterialParameterType::static_type(),
                                           []() -> MaterialParameter* { return new MaterialParameterType(); }));
        }

        friend class MaterialInstance;
    };

    class ENGINE_EXPORT MaterialInstance : public MaterialInterface
    {
        declare_class(MaterialInstance, MaterialInterface);

    private:
        Pointer<Material> _M_material = nullptr;
        MaterialParameters _M_parameters;

        MaterialInstance(Material* material = nullptr);

    public:
        const Vector<VertexBufferStream>& streams() const override;
        Pipeline* pipeline() override;
        Material* material() override;
        const Material* material() const override;

        bool apply() const override;
        MaterialInstance& rebuild();

        const MaterialParameters& parameters() const override;


        friend class Object;
        friend class Material;
    };

}// namespace Engine
