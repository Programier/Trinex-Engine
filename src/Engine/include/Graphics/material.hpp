#pragma once
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Core/structures.hpp>
#include <Graphics/texture.hpp>

namespace Engine
{
    class Pipeline;

    class ENGINE_EXPORT MaterialParameter : public SerializableObject
    {
    public:
        enum Type
        {
            Bool              = 0,
            Float             = 1,
            Int               = 2,
            Vec2              = 3,
            Vec3              = 4,
            Vec4              = 5,
            Mat3              = 6,
            Mat4              = 7,
            Sampler           = 8,
            Texture2D         = 9,
            CombinedSampler2D = 10,

            __COUNT__ = 11,
        };

        Name name;

    public:
        virtual size_t size() const;
        virtual byte* data();
        virtual const byte* data() const;
        virtual Type type() const = 0;
        virtual MaterialParameter& apply(const Pipeline* pipeline);
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

    using BoolMaterialParameter  = TypedMaterialParameter<bool, MaterialParameter::Type::Bool>;
    using FloatMaterialParameter = TypedMaterialParameter<float, MaterialParameter::Type::Float>;
    using IntMaterialParameter   = TypedMaterialParameter<int32_t, MaterialParameter::Type::Int>;
    using Vec2MaterialParameter  = TypedMaterialParameter<Vector2D, MaterialParameter::Type::Vec2>;
    using Vec3MaterialParameter  = TypedMaterialParameter<Vector3D, MaterialParameter::Type::Vec3>;
    using Vec4MaterialParameter  = TypedMaterialParameter<Vector4D, MaterialParameter::Type::Vec4>;
    using Mat3MaterialParameter  = TypedMaterialParameter<Matrix3f, MaterialParameter::Type::Mat3>;
    using Mat4MaterialParameter  = TypedMaterialParameter<Matrix4f, MaterialParameter::Type::Mat4>;

    class ENGINE_EXPORT BindingMaterialParameter : public MaterialParameter
    {
    public:
        BindLocation location;

        bool archive_process(Archive& ar) override;
    };

    class SamplerMaterialParameter : public BindingMaterialParameter
    {
    public:
        Pointer<class Sampler> sampler;

        Type type() const override;
        MaterialParameter& apply(const Pipeline* pipeline) override;
        bool archive_process(Archive& ar) override;
    };

    class CombinedSampler2DMaterialParameter : public BindingMaterialParameter
    {
    public:
        Pointer<class Sampler> sampler;
        Pointer<class Texture2D> texture;

        Type type() const override;
        MaterialParameter& apply(const Pipeline* pipeline) override;
        bool archive_process(Archive& ar) override;
    };

    struct Texture2DMaterialParameter : public BindingMaterialParameter {
        Pointer<class Texture2D> texture;

        Type type() const override;
        MaterialParameter& apply(const Pipeline* pipeline) override;
        bool archive_process(Archive& ar) override;
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
        virtual bool apply();
    };

    class ENGINE_EXPORT Material : public MaterialInterface
    {
        declare_class(Material, MaterialInterface);

    private:
        Map<Name, MaterialParameter*, Name::HashFunction> _M_material_parameters;

    protected:
        MaterialParameter* create_parameter_internal(const Name& name, MaterialParameter::Type type) override;


        void apply_shader_global_params(class Shader* shader, MaterialInterface* head);

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

        bool apply() override;
        bool apply(MaterialInterface* head);
        class Material* material() override;
        Material& apply_changes() override;


        ~Material();
    };

    class ENGINE_EXPORT MaterialInstance : public MaterialInterface
    {
        declare_class(MaterialInstance, MaterialInterface);

    private:
        Map<Name, MaterialParameter*, Name::HashFunction> _M_material_parameters;

    protected:
        MaterialParameter* create_parameter_internal(const Name& name, MaterialParameter::Type type) override;

    public:
        MaterialInterface* parent_material = nullptr;

        bool archive_process(Archive& archive) override;
        MaterialParameter* find_parameter(const Name& name) const override;
        MaterialInterface* parent() const override;
        bool apply() override;
        class Material* material() override;
    };
}// namespace Engine
