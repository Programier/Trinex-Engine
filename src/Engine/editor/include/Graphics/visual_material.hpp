#pragma once
#include <Graphics/material.hpp>

namespace Engine
{
    class ShaderCompiler;

    enum class MaterialNodeDataType
    {
        Undefined = 0,
        Bool      = BIT(0),
        Int       = BIT(1),
        UInt      = BIT(2),
        Float     = BIT(3),
        BVec2     = BIT(4),
        BVec3     = BIT(5),
        BVec4     = BIT(6),
        IVec2     = BIT(7),
        IVec3     = BIT(8),
        IVec4     = BIT(9),
        UVec2     = BIT(10),
        UVec3     = BIT(11),
        UVec4     = BIT(12),
        Vec2      = BIT(13),
        Vec3      = BIT(14),
        Vec4      = BIT(15),
        Color3    = BIT(16) | Vec3,
        Color4    = BIT(17) | Vec4,
    };

    enum class MaterialPinType
    {
        Input  = 0,
        Output = 1,
    };

    struct MaterialPin {
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
    };

    struct MaterialOutputPin : public MaterialPin {
        Set<MaterialInputPin*> linked_to;

        using MaterialPin::MaterialPin;
        MaterialPinType type() const override;
    };

    struct MaterialNode {
        static const size_t compile_error;

        Vector<MaterialInputPin*> inputs;
        Vector<MaterialOutputPin*> outputs;

        Vector2D position;

        virtual size_t compile(ShaderCompiler* compiler, MaterialOutputPin* pin);
        virtual const char* name() const;
        virtual MaterialNodeDataType output_type(const MaterialOutputPin* pin) const;

        Identifier id() const;
        virtual class Struct* struct_instance() const;
        virtual ~MaterialNode();
    };

    template<typename Type, MaterialNodeDataType enum_value>
    struct TypedInputPin : public MaterialInputPin {
        Type value;

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
    };


    template<typename Type, MaterialNodeDataType enum_value>
    struct TypedOutputPin : public MaterialOutputPin {
        Type value;

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
            if (node->inputs.empty())
                return enum_value;
            return node->output_type(this);
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


    class VisualMaterial : public Material
    {
        declare_class(VisualMaterial, Material);

    private:
        MaterialNode* _M_vertex_node   = nullptr;
        MaterialNode* _M_fragment_node = nullptr;
        Vector<MaterialNode*> _M_nodes;


    public:
        MaterialNode* vertex_node() const;
        MaterialNode* fragment_node() const;
        const Vector<MaterialNode*>& nodes() const;
        MaterialNode* create_node(class Struct*);

        VisualMaterial();
        VisualMaterial& render_nodes(void* context);
        ~VisualMaterial();
    };

}// namespace Engine
