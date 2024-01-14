#pragma once
#include <Graphics/material.hpp>

namespace Engine
{

    struct VisualMaterialElement {
        Identifier id;
        class VisualMaterial* material = nullptr;
        Vector2D position;

        virtual VisualMaterialElement& update_id();
        virtual VisualMaterialElement& init();
        virtual bool is_removable_element();
        virtual ~VisualMaterialElement();
    };

    struct Node : VisualMaterialElement {
        Vector<struct InputPin*> input;
        Vector<struct OutputPin*> output;

        // Render data

        Node& init();
        virtual Node& update_id();
        virtual const char* name() const          = 0;
        virtual EnumerateType type() const        = 0;
        virtual class Struct* node_struct() const = 0;
        virtual ~Node();
    };

    struct NodePin : VisualMaterialElement {
        enum DataType : EnumerateType
        {
            Bool   = BIT(0),
            Int    = BIT(1),
            UInt   = BIT(2),
            Float  = BIT(3),
            BVec2  = BIT(4),
            BVec3  = BIT(5),
            BVec4  = BIT(6),
            IVec2  = BIT(7),
            IVec3  = BIT(8),
            IVec4  = BIT(9),
            UVec2  = BIT(10),
            UVec3  = BIT(11),
            UVec4  = BIT(12),
            Vec2   = BIT(13),
            Vec3   = BIT(14),
            Vec4   = BIT(15),
            Color3 = BIT(16),
            Color4 = BIT(17),

            All = ~static_cast<EnumerateType>(0)
        };

        enum PinType
        {
            Input,
            Output,
        };

        EnumerateType data_types;
        Name name;
        struct Node* node = nullptr;

        NodePin(struct Node*, Name name, EnumerateType data);
        virtual void* default_value();
        virtual bool is_input_pin() const;
        virtual bool is_output_pin() const;
        virtual PinType type() const = 0;
    };

    struct OutputPin : public NodePin {
        Set<InputPin*> linked_to;

        using NodePin::NodePin;

        bool is_output_pin() const override;
        PinType type() const override;
    };

    struct InputPin : public NodePin {
        struct OutputPin* linked_to = nullptr;
        using NodePin::NodePin;

        bool is_input_pin() const override;
        PinType type() const override;
    };

    template<typename Type, auto enum_value>
    struct TypedInputPin : public InputPin {
        Type value = Type();

        TypedInputPin(struct Node* node, Name name) : InputPin(node, name, enum_value)
        {}

        void* default_value() override
        {
            if (linked_to)
                return nullptr;

            return &value;
        }
    };

    template<typename Type, auto enum_value>
    struct TypedOutputPin : public OutputPin {
        Type value = Type();

        TypedOutputPin(struct Node* node, Name name) : OutputPin(node, name, enum_value)
        {}

        void* default_value() override
        {
            if (node->input.empty())
                return &value;
            return nullptr;
        }
    };

    using BoolInputPin   = TypedInputPin<bool, NodePin::Bool>;
    using IntInputPin    = TypedInputPin<int_t, NodePin::Int>;
    using UIntInputPin   = TypedInputPin<uint_t, NodePin::UInt>;
    using FloatInputPin  = TypedInputPin<float, NodePin::Float>;
    using BVec2InputPin  = TypedInputPin<BoolVector2D, NodePin::BVec2>;
    using BVec3InputPin  = TypedInputPin<BoolVector3D, NodePin::BVec3>;
    using BVec4InputPin  = TypedInputPin<BoolVector4D, NodePin::BVec4>;
    using IVec2InputPin  = TypedInputPin<IntVector2D, NodePin::IVec2>;
    using IVec3InputPin  = TypedInputPin<IntVector3D, NodePin::IVec3>;
    using IVec4InputPin  = TypedInputPin<IntVector4D, NodePin::IVec4>;
    using UVec2InputPin  = TypedInputPin<UIntVector2D, NodePin::UVec2>;
    using UVec3InputPin  = TypedInputPin<UIntVector3D, NodePin::UVec3>;
    using UVec4InputPin  = TypedInputPin<UIntVector4D, NodePin::UVec4>;
    using Vec2InputPin   = TypedInputPin<Vector2D, NodePin::Vec2>;
    using Vec3InputPin   = TypedInputPin<Vector3D, NodePin::Vec3>;
    using Vec4InputPin   = TypedInputPin<Vector4D, NodePin::Vec4>;
    using Color3InputPin = TypedInputPin<Vector4D, NodePin::Color3>;
    using Color4InputPin = TypedInputPin<Vector4D, NodePin::Color4>;

    using BoolOutputPin   = TypedOutputPin<bool, NodePin::Bool>;
    using IntOutputPin    = TypedOutputPin<int_t, NodePin::Int>;
    using UIntOutputPin   = TypedOutputPin<uint_t, NodePin::UInt>;
    using FloatOutputPin  = TypedOutputPin<float, NodePin::Float>;
    using BVec2OutputPin  = TypedOutputPin<BoolVector2D, NodePin::BVec2>;
    using BVec3OutputPin  = TypedOutputPin<BoolVector3D, NodePin::BVec3>;
    using BVec4OutputPin  = TypedOutputPin<BoolVector4D, NodePin::BVec4>;
    using IVec2OutputPin  = TypedOutputPin<IntVector2D, NodePin::IVec2>;
    using IVec3OutputPin  = TypedOutputPin<IntVector3D, NodePin::IVec3>;
    using IVec4OutputPin  = TypedOutputPin<IntVector4D, NodePin::IVec4>;
    using UVec2OutputPin  = TypedOutputPin<UIntVector2D, NodePin::UVec2>;
    using UVec3OutputPin  = TypedOutputPin<UIntVector3D, NodePin::UVec3>;
    using UVec4OutputPin  = TypedOutputPin<UIntVector4D, NodePin::UVec4>;
    using Vec2OutputPin   = TypedOutputPin<Vector2D, NodePin::Vec2>;
    using Vec3OutputPin   = TypedOutputPin<Vector3D, NodePin::Vec3>;
    using Vec4OutputPin   = TypedOutputPin<Vector4D, NodePin::Vec4>;
    using Color3OutputPin = TypedOutputPin<Vector4D, NodePin::Color3>;
    using Color4OutputPin = TypedOutputPin<Vector4D, NodePin::Color4>;


    class VisualMaterial : public Material
    {
        declare_class(VisualMaterial, Material);

    private:
        Node* _M_root_node = nullptr;
        Set<Node*> _M_nodes;


        VisualMaterial& on_element_created(VisualMaterialElement* element);

    public:
        Node* root_node() const;
        const Set<Node*>& nodes() const;
        Identifier next_id();
        Node* create_node(class Struct*);


        template<typename Type, typename... Args>
        Type* create_element(Args&&... args)
        {
            Type* element = new Type(std::forward<Args>(args)...);
            if constexpr (std::is_base_of_v<Node, Type>)
            {
                _M_nodes.insert(element);
            }
            on_element_created(element);
            return element;
        }

        VisualMaterial();
        ~VisualMaterial();

        friend struct Node;
    };

}// namespace Engine
