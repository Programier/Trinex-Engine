#pragma once
#include <Core/archive.hpp>
#include <Graphics/material.hpp>

namespace Engine
{
    class ShaderCompiler;

    enum class MaterialBaseDataType
    {
        Void    = 0,
        Bool    = 1,
        Int     = 2,
        UInt    = 3,
        Float   = 4,
        Color   = 5,
        Sampler = 6,
    };


    constexpr inline size_t material_type_value(MaterialBaseDataType bt, size_t component_count, bool is_convertable = true)
    {
        return (static_cast<size_t>(is_convertable) & 0b1) | (static_cast<size_t>(bt) << 1) | (component_count << 4);
    }

    enum class MaterialNodeDataType : size_t
    {
        Undefined = ~static_cast<size_t>(0),
        Bool      = material_type_value(MaterialBaseDataType::Bool, 1),
        Int       = material_type_value(MaterialBaseDataType::Int, 1),
        UInt      = material_type_value(MaterialBaseDataType::UInt, 1),
        Float     = material_type_value(MaterialBaseDataType::Float, 1),
        BVec2     = material_type_value(MaterialBaseDataType::Bool, 2),
        BVec3     = material_type_value(MaterialBaseDataType::Bool, 3),
        BVec4     = material_type_value(MaterialBaseDataType::Bool, 4),
        IVec2     = material_type_value(MaterialBaseDataType::Int, 2),
        IVec3     = material_type_value(MaterialBaseDataType::Int, 3),
        IVec4     = material_type_value(MaterialBaseDataType::Int, 4),
        UVec2     = material_type_value(MaterialBaseDataType::UInt, 2),
        UVec3     = material_type_value(MaterialBaseDataType::UInt, 3),
        UVec4     = material_type_value(MaterialBaseDataType::UInt, 4),
        Vec2      = material_type_value(MaterialBaseDataType::Float, 2),
        Vec3      = material_type_value(MaterialBaseDataType::Float, 3),
        Vec4      = material_type_value(MaterialBaseDataType::Float, 4),
        Color3    = material_type_value(MaterialBaseDataType::Color, 3),
        Color4    = material_type_value(MaterialBaseDataType::Color, 4),
        Mat3      = material_type_value(MaterialBaseDataType::Float, 9),
        Mat4      = material_type_value(MaterialBaseDataType::Float, 16),
        Sampler   = material_type_value(MaterialBaseDataType::Sampler, 1, false),
    };


    struct MaterialDataTypeInfo {
        MaterialBaseDataType base_type;
        size_t components_count;
        bool is_convertable;


        FORCE_INLINE bool is_matrix() const
        {
            return components_count == 9 || components_count == 16;
        }

        FORCE_INLINE size_t matrix_size() const
        {
            if (!is_matrix())
                return 0;

            return components_count == 9 ? 3 : 4;
        }

        static MaterialDataTypeInfo from(MaterialNodeDataType type);
    };


    MaterialNodeDataType calculate_operator_types(MaterialNodeDataType& t1, MaterialNodeDataType& t2);
    MaterialNodeDataType operator_result_between(MaterialNodeDataType t1, MaterialNodeDataType t2);
    bool is_equal_types(MaterialNodeDataType type1, MaterialNodeDataType type2);

    enum class MaterialPinType
    {
        Input  = 0,
        Output = 1,
    };

    struct MaterialPin : public SerializableObject {
        Name name;
        struct MaterialNode* node = nullptr;

        MaterialPin(MaterialNode* node, Name name = Name::none);

        Identifier id() const;
        bool is_input_pin() const;
        bool is_output_pin() const;
        virtual void* default_value();
        virtual MaterialNodeDataType value_type() const;
        virtual MaterialPinType type() const = 0;
        virtual ~MaterialPin();
    };

    struct MaterialInputPin : public MaterialPin {
        struct MaterialOutputPin* linked_to = nullptr;


        using MaterialPin::MaterialPin;
        MaterialPinType type() const override;
        MaterialNodeDataType value_type() const override;
        MaterialPin& link_to(MaterialOutputPin* pin);
    };

    struct MaterialOutputPin : public MaterialPin {
        Set<MaterialInputPin*> linked_to;

        using MaterialPin::MaterialPin;
        MaterialPinType type() const override;
        MaterialNodeDataType value_type() const override;
        size_t refereces_count() const;
    };

    struct MaterialNode : public SerializableObject {
        static const size_t compile_error;

        Vector<MaterialInputPin*> inputs;
        Vector<MaterialOutputPin*> outputs;
        class VisualMaterial* material = nullptr;

        Vector2D position = {0, 0};

        virtual void render();
        virtual bool is_removable() const;
        virtual size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin);
        virtual const char* name() const;
        virtual MaterialNodeDataType output_type(const MaterialOutputPin* pin) const;
        bool archive_process(Archive& ar) override;

        Index pin_index(MaterialOutputPin* pin) const;
        Index pin_index(MaterialInputPin* pin) const;

        Identifier id() const;
        virtual class Struct* struct_instance() const;
        virtual ~MaterialNode();
    };


    template<MaterialNodeDataType enum_value>
    struct TypedInputNoDefaultPin : public MaterialInputPin {
        static constexpr MaterialNodeDataType data_type = enum_value;

        TypedInputNoDefaultPin(struct MaterialNode* node, Name name = Name::none) : MaterialInputPin(node, name)
        {}

        MaterialNodeDataType value_type() const override
        {
            if (linked_to)
                return linked_to->value_type();
            return enum_value;
        }
    };

    template<typename Type, MaterialNodeDataType enum_value>
    struct TypedInputPin : public MaterialInputPin {
        Type value;

        using NativeType                                = Type;
        static constexpr MaterialNodeDataType data_type = enum_value;

        TypedInputPin(struct MaterialNode* node, Name name = Name::none, const Type& default_value = Type())
            : MaterialInputPin(node, name), value(default_value)
        {}

        void* default_value() override
        {
            if (linked_to)
                return nullptr;
            return &value;
        }

        MaterialNodeDataType value_type() const override
        {
            if (linked_to)
            {
                return linked_to->value_type();
            }
            return enum_value;
        }

        bool archive_process(Archive& ar) override
        {
            if (!MaterialInputPin::archive_process(ar))
                return false;

            ar & value;
            return ar;
        }
    };

    template<MaterialNodeDataType enum_value>
    struct TypedOutputNoDefaultPin : public MaterialOutputPin {
        static constexpr MaterialNodeDataType data_type = enum_value;

        TypedOutputNoDefaultPin(struct MaterialNode* node, Name name = Name::none) : MaterialOutputPin(node, name)
        {}

        MaterialNodeDataType value_type() const override
        {
            return enum_value;
        }
    };


    template<typename Type, MaterialNodeDataType enum_value>
    struct TypedOutputPin : public MaterialOutputPin {
        Type value;

        using NativeType                                = Type;
        static constexpr MaterialNodeDataType data_type = enum_value;

        TypedOutputPin(struct MaterialNode* node, Name name = Name::none, const Type& default_value = Type())
            : MaterialOutputPin(node, name), value(default_value)
        {}

        void* default_value() override
        {
            if (node->inputs.empty())
                return &value;
            return nullptr;
        }

        MaterialNodeDataType value_type() const override
        {
            return enum_value;
        }

        bool archive_process(Archive& ar) override
        {
            if (!MaterialOutputPin::archive_process(ar))
                return false;

            ar & value;
            return ar;
        }
    };

    using BoolInputPin   = TypedInputPin<bool, MaterialNodeDataType::Bool>;
    using IntInputPin    = TypedInputPin<int_t, MaterialNodeDataType::Int>;
    using UIntInputPin   = TypedInputPin<uint_t, MaterialNodeDataType::UInt>;
    using FloatInputPin  = TypedInputPin<float, MaterialNodeDataType::Float>;
    using BVec2InputPin  = TypedInputPin<BoolVector2D, MaterialNodeDataType::BVec2>;
    using BVec3InputPin  = TypedInputPin<BoolVector3D, MaterialNodeDataType::BVec3>;
    using BVec4InputPin  = TypedInputPin<BoolVector4D, MaterialNodeDataType::BVec4>;
    using IVec2InputPin  = TypedInputPin<IntVector2D, MaterialNodeDataType::IVec2>;
    using IVec3InputPin  = TypedInputPin<IntVector3D, MaterialNodeDataType::IVec3>;
    using IVec4InputPin  = TypedInputPin<IntVector4D, MaterialNodeDataType::IVec4>;
    using UVec2InputPin  = TypedInputPin<UIntVector2D, MaterialNodeDataType::UVec2>;
    using UVec3InputPin  = TypedInputPin<UIntVector3D, MaterialNodeDataType::UVec3>;
    using UVec4InputPin  = TypedInputPin<UIntVector4D, MaterialNodeDataType::UVec4>;
    using Vec2InputPin   = TypedInputPin<Vector2D, MaterialNodeDataType::Vec2>;
    using Vec3InputPin   = TypedInputPin<Vector3D, MaterialNodeDataType::Vec3>;
    using Vec4InputPin   = TypedInputPin<Vector4D, MaterialNodeDataType::Vec4>;
    using Color3InputPin = TypedInputPin<Vector3D, MaterialNodeDataType::Color3>;
    using Color4InputPin = TypedInputPin<Vector4D, MaterialNodeDataType::Color4>;

    using Mat3InputPin = TypedInputPin<Matrix3f, MaterialNodeDataType::Mat3>;
    using Mat4InputPin = TypedInputPin<Matrix4f, MaterialNodeDataType::Mat4>;

    using BoolInputNoDefaultPin    = TypedInputNoDefaultPin<MaterialNodeDataType::Bool>;
    using IntInputNoDefaultPin     = TypedInputNoDefaultPin<MaterialNodeDataType::Int>;
    using UIntInputNoDefaultPin    = TypedInputNoDefaultPin<MaterialNodeDataType::UInt>;
    using FloatInputNoDefaultPin   = TypedInputNoDefaultPin<MaterialNodeDataType::Float>;
    using BVec2InputNoDefaultPin   = TypedInputNoDefaultPin<MaterialNodeDataType::BVec2>;
    using BVec3InputNoDefaultPin   = TypedInputNoDefaultPin<MaterialNodeDataType::BVec3>;
    using BVec4InputNoDefaultPin   = TypedInputNoDefaultPin<MaterialNodeDataType::BVec4>;
    using IVec2InputNoDefaultPin   = TypedInputNoDefaultPin<MaterialNodeDataType::IVec2>;
    using IVec3InputNoDefaultPin   = TypedInputNoDefaultPin<MaterialNodeDataType::IVec3>;
    using IVec4InputNoDefaultPin   = TypedInputNoDefaultPin<MaterialNodeDataType::IVec4>;
    using UVec2InputNoDefaultPin   = TypedInputNoDefaultPin<MaterialNodeDataType::UVec2>;
    using UVec3InputNoDefaultPin   = TypedInputNoDefaultPin<MaterialNodeDataType::UVec3>;
    using UVec4InputNoDefaultPin   = TypedInputNoDefaultPin<MaterialNodeDataType::UVec4>;
    using Vec2InputNoDefaultPin    = TypedInputNoDefaultPin<MaterialNodeDataType::Vec2>;
    using Vec3InputNoDefaultPin    = TypedInputNoDefaultPin<MaterialNodeDataType::Vec3>;
    using Vec4InputNoDefaultPin    = TypedInputNoDefaultPin<MaterialNodeDataType::Vec4>;
    using Color3InputNoDefaultPin  = TypedInputNoDefaultPin<MaterialNodeDataType::Color3>;
    using Color4InputNoDefaultPin  = TypedInputNoDefaultPin<MaterialNodeDataType::Color4>;
    using Mat3InputNoDefaultPin    = TypedInputNoDefaultPin<MaterialNodeDataType::Mat3>;
    using Mat4InputNoDefaultPin    = TypedInputNoDefaultPin<MaterialNodeDataType::Mat4>;
    using SamplerInputNoDefaultPin = TypedInputNoDefaultPin<MaterialNodeDataType::Sampler>;

    using BoolOutputPin   = TypedOutputPin<bool, MaterialNodeDataType::Bool>;
    using IntOutputPin    = TypedOutputPin<int_t, MaterialNodeDataType::Int>;
    using UIntOutputPin   = TypedOutputPin<uint_t, MaterialNodeDataType::UInt>;
    using FloatOutputPin  = TypedOutputPin<float, MaterialNodeDataType::Float>;
    using BVec2OutputPin  = TypedOutputPin<BoolVector2D, MaterialNodeDataType::BVec2>;
    using BVec3OutputPin  = TypedOutputPin<BoolVector3D, MaterialNodeDataType::BVec3>;
    using BVec4OutputPin  = TypedOutputPin<BoolVector4D, MaterialNodeDataType::BVec4>;
    using IVec2OutputPin  = TypedOutputPin<IntVector2D, MaterialNodeDataType::IVec2>;
    using IVec3OutputPin  = TypedOutputPin<IntVector3D, MaterialNodeDataType::IVec3>;
    using IVec4OutputPin  = TypedOutputPin<IntVector4D, MaterialNodeDataType::IVec4>;
    using UVec2OutputPin  = TypedOutputPin<UIntVector2D, MaterialNodeDataType::UVec2>;
    using UVec3OutputPin  = TypedOutputPin<UIntVector3D, MaterialNodeDataType::UVec3>;
    using UVec4OutputPin  = TypedOutputPin<UIntVector4D, MaterialNodeDataType::UVec4>;
    using Vec2OutputPin   = TypedOutputPin<Vector2D, MaterialNodeDataType::Vec2>;
    using Vec3OutputPin   = TypedOutputPin<Vector3D, MaterialNodeDataType::Vec3>;
    using Vec4OutputPin   = TypedOutputPin<Vector4D, MaterialNodeDataType::Vec4>;
    using Color3OutputPin = TypedOutputPin<Vector3D, MaterialNodeDataType::Color3>;
    using Color4OutputPin = TypedOutputPin<Vector4D, MaterialNodeDataType::Color4>;
    using Mat3OutputPin   = TypedOutputPin<Matrix3f, MaterialNodeDataType::Mat3>;
    using Mat4OutputPin   = TypedOutputPin<Matrix4f, MaterialNodeDataType::Mat4>;

    using BoolOutputNoDefaultPin    = TypedOutputNoDefaultPin<MaterialNodeDataType::Bool>;
    using IntOutputNoDefaultPin     = TypedOutputNoDefaultPin<MaterialNodeDataType::Int>;
    using UIntOutputNoDefaultPin    = TypedOutputNoDefaultPin<MaterialNodeDataType::UInt>;
    using FloatOutputNoDefaultPin   = TypedOutputNoDefaultPin<MaterialNodeDataType::Float>;
    using BVec2OutputNoDefaultPin   = TypedOutputNoDefaultPin<MaterialNodeDataType::BVec2>;
    using BVec3OutputNoDefaultPin   = TypedOutputNoDefaultPin<MaterialNodeDataType::BVec3>;
    using BVec4OutputNoDefaultPin   = TypedOutputNoDefaultPin<MaterialNodeDataType::BVec4>;
    using IVec2OutputNoDefaultPin   = TypedOutputNoDefaultPin<MaterialNodeDataType::IVec2>;
    using IVec3OutputNoDefaultPin   = TypedOutputNoDefaultPin<MaterialNodeDataType::IVec3>;
    using IVec4OutputNoDefaultPin   = TypedOutputNoDefaultPin<MaterialNodeDataType::IVec4>;
    using UVec2OutputNoDefaultPin   = TypedOutputNoDefaultPin<MaterialNodeDataType::UVec2>;
    using UVec3OutputNoDefaultPin   = TypedOutputNoDefaultPin<MaterialNodeDataType::UVec3>;
    using UVec4OutputNoDefaultPin   = TypedOutputNoDefaultPin<MaterialNodeDataType::UVec4>;
    using Vec2OutputNoDefaultPin    = TypedOutputNoDefaultPin<MaterialNodeDataType::Vec2>;
    using Vec3OutputNoDefaultPin    = TypedOutputNoDefaultPin<MaterialNodeDataType::Vec3>;
    using Vec4OutputNoDefaultPin    = TypedOutputNoDefaultPin<MaterialNodeDataType::Vec4>;
    using Color3OutputNoDefaultPin  = TypedOutputNoDefaultPin<MaterialNodeDataType::Color3>;
    using Color4OutputNoDefaultPin  = TypedOutputNoDefaultPin<MaterialNodeDataType::Color4>;
    using Mat3OutputNoDefaultPin    = TypedOutputNoDefaultPin<MaterialNodeDataType::Mat3>;
    using Mat4OutputNoDefaultPin    = TypedOutputNoDefaultPin<MaterialNodeDataType::Mat4>;
    using SamplerOutputNoDefaultPin = TypedOutputNoDefaultPin<MaterialNodeDataType::Sampler>;

    class VisualMaterial : public Material
    {
        declare_class(VisualMaterial, Material);

    private:
        MaterialNode* m_vertex_node   = nullptr;
        MaterialNode* m_fragment_node = nullptr;
        Vector<MaterialNode*> m_nodes;


    public:
        MaterialNode* vertex_node() const;
        MaterialNode* fragment_node() const;
        const Vector<MaterialNode*>& nodes() const;
        MaterialNode* create_node(class Struct*, const Vector2D& position = {});
        Index node_index(const MaterialNode* node) const;


        VisualMaterial();
        VisualMaterial& render_nodes(void* context);
        ~VisualMaterial();
        bool archive_process(Archive& ar) override;


        template<typename Type>
        Type* create_node(const Vector2D& position = {})
        {
            Type* node = new Type();
            m_nodes.push_back(node);
            node->material = this;
            node->position = position;
            return node;
        }

        friend struct MaterialNode;
    };

}// namespace Engine
