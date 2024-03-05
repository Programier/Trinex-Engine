#pragma once
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Core/structures.hpp>
#include <Graphics/texture.hpp>

namespace Engine
{
    class Pipeline;
    class SceneComponent;
    class Texture2D;
    class Sampler;

    class ENGINE_EXPORT MaterialParameter : public SerializableObject
    {
    public:
        enum Type
        {
            Bool               = 0,
            Int                = 1,
            UInt               = 2,
            Float              = 3,
            BVec2              = 4,
            BVec3              = 5,
            BVec4              = 6,
            IVec2              = 7,
            IVec3              = 8,
            IVec4              = 9,
            UVec2              = 10,
            UVec3              = 11,
            UVec4              = 12,
            Vec2               = 13,
            Vec3               = 14,
            Vec4               = 15,
            Mat3               = 16,
            Mat4               = 17,
            Sampler            = 18,
            Texture2D          = 19,
            CombinedTexture2D  = 20,
            ModelMatrix        = 21,
            BaseColorTexture   = 22,
            PositionTexture    = 23,
            NormalTexture      = 24,
            EmissiveTexture    = 25,
            MSRABufferTexture  = 26,
            SceneOutputTexture = 27,

            __COUNT__ = 28
        };

        Name name;

    public:
        virtual size_t size() const;
        virtual byte* data();
        virtual const byte* data() const;
        virtual Type type() const = 0;
        virtual Type binding_object_type() const;
        virtual MaterialParameter& apply(const Pipeline* pipeline, SceneComponent* component);
        virtual bool archive_process(Archive& ar) override;

        template<typename T>
        T* get()
        {
            if (size() == sizeof(T))
            {
                return reinterpret_cast<T*>(data());
            }
            return nullptr;
        }

        template<typename T>
        const T* get() const
        {
            if (size() == sizeof(T))
            {
                return reinterpret_cast<const T*>(data());
            }
            return nullptr;
        }

        virtual ~MaterialParameter() = default;
    };

    template<typename TypeData, MaterialParameter::Type _parameter_type>
    class TypedMaterialParameter : public MaterialParameter
    {
    public:
        TypeData param;
        static constexpr inline MaterialParameter::Type parameter_type = _parameter_type;
        TypedMaterialParameter(TypeData value = TypeData()) : param(value)
        {}

        MaterialParameter::Type type() const override
        {
            return parameter_type;
        }

        size_t size() const override
        {
            return sizeof(TypeData);
        }

        byte* data() override
        {
            return reinterpret_cast<byte*>(&param);
        }

        const byte* data() const override
        {
            return reinterpret_cast<const byte*>(&param);
        }
    };

    template<typename TypeData, MaterialParameter::Type _parameter_type>
    class TypedMatrixMaterialParameter : public TypedMaterialParameter<TypeData, _parameter_type>
    {
    public:
        TypedMatrixMaterialParameter(TypeData value = TypeData(1.f)) : TypedMaterialParameter<TypeData, _parameter_type>(value)
        {}
    };

    using BoolMaterialParameter  = TypedMaterialParameter<bool, MaterialParameter::Type::Bool>;
    using IntMaterialParameter   = TypedMaterialParameter<int32_t, MaterialParameter::Type::Int>;
    using UIntMaterialParameter  = TypedMaterialParameter<uint32_t, MaterialParameter::Type::UInt>;
    using FloatMaterialParameter = TypedMaterialParameter<float, MaterialParameter::Type::Float>;

    using BVec2MaterialParameter = TypedMaterialParameter<BoolVector2D, MaterialParameter::Type::BVec2>;
    using BVec3MaterialParameter = TypedMaterialParameter<BoolVector3D, MaterialParameter::Type::BVec3>;
    using BVec4MaterialParameter = TypedMaterialParameter<BoolVector4D, MaterialParameter::Type::BVec4>;

    using IVec2MaterialParameter = TypedMaterialParameter<IntVector2D, MaterialParameter::Type::IVec2>;
    using IVec3MaterialParameter = TypedMaterialParameter<IntVector3D, MaterialParameter::Type::IVec3>;
    using IVec4MaterialParameter = TypedMaterialParameter<IntVector4D, MaterialParameter::Type::IVec4>;

    using UVec2MaterialParameter = TypedMaterialParameter<UIntVector2D, MaterialParameter::Type::UVec2>;
    using UVec3MaterialParameter = TypedMaterialParameter<UIntVector3D, MaterialParameter::Type::UVec3>;
    using UVec4MaterialParameter = TypedMaterialParameter<UIntVector4D, MaterialParameter::Type::UVec4>;

    using Vec2MaterialParameter = TypedMaterialParameter<Vector2D, MaterialParameter::Type::Vec2>;
    using Vec3MaterialParameter = TypedMaterialParameter<Vector3D, MaterialParameter::Type::Vec3>;
    using Vec4MaterialParameter = TypedMaterialParameter<Vector4D, MaterialParameter::Type::Vec4>;
    using Mat3MaterialParameter = TypedMatrixMaterialParameter<Matrix3f, MaterialParameter::Type::Mat3>;
    using Mat4MaterialParameter = TypedMatrixMaterialParameter<Matrix4f, MaterialParameter::Type::Mat4>;

    class ENGINE_EXPORT BindingMaterialParameter : public MaterialParameter
    {
    public:
        BindLocation location;

    protected:
        void bind_texture(class Engine::Texture2D* texture);
        void bind_sampler(class Engine::Sampler* sampler);
        void bind_combined(class Engine::Sampler* sampler, class Engine::Texture2D* texture);
        bool archive_process(Archive& ar) override;

    public:
        virtual class Texture* texture_param() const;
        virtual class Sampler* sampler_param() const;
        virtual BindingMaterialParameter& texture_param(class Texture* texture);
        virtual BindingMaterialParameter& sampler_param(class Sampler* sampler);
    };

    class SamplerMaterialParameter : public BindingMaterialParameter
    {
    public:
        Pointer<class Sampler> sampler;

        Type type() const override;
        MaterialParameter& apply(const Pipeline* pipeline, SceneComponent* component = nullptr) override;
        bool archive_process(Archive& ar) override;
        class Sampler* sampler_param() const override;
        SamplerMaterialParameter& sampler_param(class Sampler* sampler) override;
    };

    class CombinedTexture2DMaterialParameter : public BindingMaterialParameter
    {
    public:
        Pointer<class Sampler> sampler;
        Pointer<class Texture2D> texture;

        Type type() const override;
        MaterialParameter& apply(const Pipeline* pipeline, SceneComponent* component = nullptr) override;
        bool archive_process(Archive& ar) override;

        class Texture* texture_param() const override;
        class Sampler* sampler_param() const override;
        CombinedTexture2DMaterialParameter& texture_param(class Texture* texture) override;
        CombinedTexture2DMaterialParameter& sampler_param(class Sampler* sampler) override;
    };

    struct Texture2DMaterialParameter : public BindingMaterialParameter {
        Pointer<class Texture2D> texture;

        Type type() const override;
        MaterialParameter& apply(const Pipeline* pipeline, SceneComponent* component = nullptr) override;
        bool archive_process(Archive& ar) override;
        class Texture* texture_param() const override;
        Texture2DMaterialParameter& texture_param(class Texture* texture) override;
    };

    struct ModelMatrixMaterialParameter : public MaterialParameter {
        Type type() const override;
        ModelMatrixMaterialParameter& apply(const Pipeline* pipeline, SceneComponent* component = nullptr) override;
    };


    struct BaseColorTextureMaterialParameter : SamplerMaterialParameter {
        Type type() const override;
        Type binding_object_type() const override;
        BaseColorTextureMaterialParameter& apply(const Pipeline* pipeline, SceneComponent* component = nullptr) override;
    };

    struct PositionTextureMaterialParameter : SamplerMaterialParameter {
        Type type() const override;
        Type binding_object_type() const override;
        PositionTextureMaterialParameter& apply(const Pipeline* pipeline, SceneComponent* component = nullptr) override;
    };

    struct NormalTextureMaterialParameter : SamplerMaterialParameter {
        Type type() const override;
        Type binding_object_type() const override;
        NormalTextureMaterialParameter& apply(const Pipeline* pipeline, SceneComponent* component = nullptr) override;
    };

    struct EmissiveTextureMaterialParameter : SamplerMaterialParameter {
        Type type() const override;
        Type binding_object_type() const override;
        EmissiveTextureMaterialParameter& apply(const Pipeline* pipeline, SceneComponent* component = nullptr) override;
    };

    struct MSRABufferTextureMaterialParameter : SamplerMaterialParameter {
        Type type() const override;
        Type binding_object_type() const override;
        MSRABufferTextureMaterialParameter& apply(const Pipeline* pipeline, SceneComponent* component = nullptr) override;
    };

    struct SceneOutputTextureMaterialParameter : SamplerMaterialParameter {
        Type type() const override;
        Type binding_object_type() const override;
        SceneOutputTextureMaterialParameter& apply(const Pipeline* pipeline, SceneComponent* component = nullptr) override;
    };


    class ENGINE_EXPORT MaterialInterface : public Object
    {
        declare_class(MaterialInterface, Object);

    protected:
        bool serialize_parameters(Map<Name, MaterialParameter*, Name::HashFunction>& map, Archive& ar);

        virtual MaterialParameter* create_parameter_internal(const Name& name, MaterialParameter::Type type) = 0;

    public:
        virtual MaterialParameter* find_parameter(const Name& name) const;
        virtual MaterialInterface* parent() const;
        virtual class Material* material();
        virtual bool apply(SceneComponent* component = nullptr);
    };

    class ENGINE_EXPORT Material : public MaterialInterface
    {
        declare_class(Material, MaterialInterface);

    private:
        Map<Name, MaterialParameter*, Name::HashFunction> m_material_parameters;

    protected:
        MaterialParameter* create_parameter_internal(const Name& name, MaterialParameter::Type type) override;


        void apply_shader_global_params(class Shader* shader, MaterialInterface* head, SceneComponent* component);

    public:
        Pipeline* pipeline;

        Material();
        bool archive_process(Archive& archive) override;
        Material& preload() override;
        Material& postload() override;

        MaterialParameter* find_parameter(const Name& name) const override;
        MaterialParameter* create_parameter(const Name& name, MaterialParameter::Type type);
        Material& remove_parameter(const Name& name);
        Material& clear_parameters();

        bool apply(SceneComponent* component = nullptr) override;
        bool apply(MaterialInterface* head, SceneComponent* component = nullptr);
        class Material* material() override;
        Material& apply_changes() override;


        ~Material();
    };

    class ENGINE_EXPORT MaterialInstance : public MaterialInterface
    {
        declare_class(MaterialInstance, MaterialInterface);

    private:
        Map<Name, MaterialParameter*, Name::HashFunction> m_material_parameters;

    protected:
        MaterialParameter* create_parameter_internal(const Name& name, MaterialParameter::Type type) override;

    public:
        MaterialInterface* parent_material = nullptr;

        bool archive_process(Archive& archive) override;
        MaterialParameter* find_parameter(const Name& name) const override;
        MaterialInterface* parent() const override;
        bool apply(SceneComponent* component = nullptr) override;
        class Material* material() override;
    };
}// namespace Engine
