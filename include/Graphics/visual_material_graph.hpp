#pragma once
#include <Core/engine_types.hpp>
#include <Core/flags.hpp>
#include <Core/object.hpp>

namespace Engine
{
    class VisualMaterial;
}

namespace Engine::VisualMaterialGraph
{
    enum class PinType : BitMask
    {
        Undefined = 0,

        Scalar  = BIT(0),
        Vector  = BIT(1),
        Numeric = Scalar | Vector,
        Matrix  = BIT(2),
        Object  = BIT(3),

        Bool                   = BIT(4) | Scalar,
        Int                    = BIT(5) | Scalar,
        UInt                   = BIT(6) | Scalar,
        Float                  = BIT(7) | Scalar,
        BVec2                  = BIT(8) | Vector,
        BVec3                  = BIT(9) | Vector,
        BVec4                  = BIT(10) | Vector,
        IVec2                  = BIT(11) | Vector,
        IVec3                  = BIT(12) | Vector,
        IVec4                  = BIT(13) | Vector,
        UVec2                  = BIT(14) | Vector,
        UVec3                  = BIT(15) | Vector,
        UVec4                  = BIT(16) | Vector,
        Vec2                   = BIT(17) | Vector,
        Vec3                   = BIT(18) | Vector,
        Color3                 = BIT(19) | Vector,
        Vec4                   = BIT(20) | Vector,
        Color4                 = BIT(21) | Vector,
        Mat3                   = BIT(22) | Matrix,
        Mat4                   = BIT(23) | Matrix,
        Sampler                = BIT(24) | Object,
        CombinedImageSampler2D = BIT(25) | Object,
        Texture2D              = BIT(26) | Object,
    };

    static FORCE_INLINE bool is_scalar(PinType type)
    {
        return (Flags<PinType>(type) & PinType::Scalar) == PinType::Scalar;
    }

    static FORCE_INLINE bool is_vector(PinType type)
    {
        return (Flags<PinType>(type) & PinType::Vector) == PinType::Vector;
    }

    static FORCE_INLINE bool is_numeric(PinType type)
    {
        return (Flags<PinType>(type) & PinType::Numeric) != 0;
    }

    static FORCE_INLINE bool is_matrix(PinType type)
    {
        return (Flags<PinType>(type) & PinType::Matrix) == PinType::Matrix;
    }

    static FORCE_INLINE bool is_object(PinType type)
    {
        return (Flags<PinType>(type) & PinType::Object) == PinType::Object;
    }

    static FORCE_INLINE bool is_convertable(PinType first, PinType second)
    {
        return (first == second || (is_numeric(first) && is_numeric(second)) || (is_matrix(first) && is_matrix(second)));
    }

    static FORCE_INLINE constexpr PinType to_int_or_float(PinType type)
    {
        switch (type)
        {
            case PinType::Bool:
                return PinType::Float;
            case PinType::BVec2:
                return PinType::Vec2;
            case PinType::BVec3:
                return PinType::Vec3;
            case PinType::BVec4:
                return PinType::Vec4;
            default:
                return type;
        }
        return type;
    }

    static FORCE_INLINE constexpr PinType to_floating_point(PinType type)
    {
        switch (type)
        {
            case PinType::Bool:
            case PinType::Int:
            case PinType::UInt:
            case PinType::Float:
                return PinType::Float;

            case PinType::BVec2:
            case PinType::IVec2:
            case PinType::UVec2:
            case PinType::Vec2:
                return PinType::Vec2;

            case PinType::BVec3:
            case PinType::IVec3:
            case PinType::UVec3:
            case PinType::Vec3:
                return PinType::Float;
            case PinType::Color3:
                return PinType::Color3;

            case PinType::BVec4:
            case PinType::IVec4:
            case PinType::UVec4:
            case PinType::Vec4:
                return PinType::Vec4;

            default:
                return type;
        }

        return type;
    }


    static FORCE_INLINE uint_t components_count(PinType type)
    {
        switch (type)
        {
            case PinType::Bool:
                return 1;
            case PinType::Int:
                return 1;
            case PinType::UInt:
                return 1;
            case PinType::Float:
                return 1;
            case PinType::BVec2:
                return 2;
            case PinType::BVec3:
                return 3;
            case PinType::BVec4:
                return 4;
            case PinType::IVec2:
                return 2;
            case PinType::IVec3:
                return 3;
            case PinType::IVec4:
                return 4;
            case PinType::UVec2:
                return 2;
            case PinType::UVec3:
                return 3;
            case PinType::UVec4:
                return 4;
            case PinType::Vec2:
                return 2;
            case PinType::Vec3:
            case PinType::Color3:
                return 3;
            case PinType::Vec4:
            case PinType::Color4:
                return 4;
            case PinType::Mat3:
                return 9;
            case PinType::Mat4:
                return 16;
            default:
                return 0;
        }
    }

    static FORCE_INLINE PinType components_type(PinType type)
    {
        switch (type)
        {
            case PinType::Bool:
            case PinType::BVec2:
            case PinType::BVec3:
            case PinType::BVec4:
                return PinType::Bool;

            case PinType::Int:
            case PinType::IVec2:
            case PinType::IVec3:
            case PinType::IVec4:
                return PinType::Int;

            case PinType::UInt:
            case PinType::UVec2:
            case PinType::UVec3:
            case PinType::UVec4:
                return PinType::UInt;

            case PinType::Float:
            case PinType::Vec2:
            case PinType::Vec3:
            case PinType::Color3:
            case PinType::Vec4:
            case PinType::Color4:
            case PinType::Mat3:
            case PinType::Mat4:
                return PinType::Float;

            default:
                return PinType::Undefined;
        }
    }


    struct ENGINE_EXPORT Expression {
        String code;
        PinType type;
        bool is_variable;

        FORCE_INLINE Expression() : code(""), type(PinType::Undefined), is_variable(false)
        {}

        FORCE_INLINE Expression(const String& code, PinType type, bool is_var = false)
            : code(code), type(type), is_variable(is_var)
        {}

        FORCE_INLINE bool is_valid() const
        {
            return type != PinType::Undefined;
        }

        FORCE_INLINE Expression& reset()
        {
            code.clear();
            type        = PinType::Undefined;
            is_variable = false;
            return *this;
        }
    };

    enum class PinKind
    {
        Input  = 0,
        Output = 1,
    };

    class Node;

    class ENGINE_EXPORT Pin
    {
    private:
        Node* m_node;
        Name m_name;

    public:
        Pin(Node* node, Name name);
        virtual ~Pin();

        FORCE_INLINE Identifier id() const
        {
            return reinterpret_cast<Identifier>(this);
        }

        Node* node() const;
        Name name() const;
        bool has_links() const;

        virtual PinKind kind() const       = 0;
        virtual size_t links_count() const = 0;
        virtual void unlink()              = 0;

        virtual PinType type() const;
        virtual void* default_value();

        template<typename Out>
        Out* default_value_as()
        {
            return reinterpret_cast<Out*>(default_value());
        }
    };

    class OutputPin;

    class ENGINE_EXPORT InputPin : public Pin
    {
        OutputPin* m_linked_to;

    public:
        InputPin(Node* node, Name name);
        PinKind kind() const override;
        size_t links_count() const override;
        void unlink() override;

        OutputPin* linked_to() const;
        void create_link(OutputPin* pin);

        friend class OutputPin;
    };

    class ENGINE_EXPORT OutputPin : public Pin
    {
        Set<InputPin*> m_linked_to;

        void link_pin_internal(InputPin* pin);
        void unlink_pin_internal(InputPin* pin);

    public:
        OutputPin(Node* node, Name name);
        PinKind kind() const override;
        size_t links_count() const override;
        void unlink() override;
        const Set<InputPin*>& linked_to() const;

        friend class InputPin;
    };

    template<PinType m_pin_type, typename BaseClass>
    class TypedPinNoDefault : public BaseClass
    {
    public:
        using BaseClass::BaseClass;

        PinType type() const
        {
            return m_pin_type;
        }
    };

    template<PinType m_pin_type, typename Value, typename BaseClass>
    class TypedPin : public TypedPinNoDefault<m_pin_type, BaseClass>
    {
    public:
        using ValueType = Value;

        ValueType value;

        TypedPin(Node* node, Name name, const ValueType& value = {})
            : TypedPinNoDefault<m_pin_type, BaseClass>(node, name), value(value)
        {}

        virtual void* default_value()
        {
            if constexpr (std::is_base_of_v<InputPin, BaseClass>)
            {
                if (Pin::has_links())
                    return nullptr;
            }
            else if constexpr (std::is_base_of_v<OutputPin, BaseClass>)
            {
                if (!Pin::node()->inputs().empty())
                    return nullptr;
            }
            return &value;
        }
    };

    using BoolInputPinND   = TypedPinNoDefault<PinType::Bool, InputPin>;
    using IntInputPinND    = TypedPinNoDefault<PinType::Int, InputPin>;
    using UIntInputPinND   = TypedPinNoDefault<PinType::UInt, InputPin>;
    using FloatInputPinND  = TypedPinNoDefault<PinType::Float, InputPin>;
    using BVec2InputPinND  = TypedPinNoDefault<PinType::BVec2, InputPin>;
    using BVec3InputPinND  = TypedPinNoDefault<PinType::BVec3, InputPin>;
    using BVec4InputPinND  = TypedPinNoDefault<PinType::BVec4, InputPin>;
    using IVec2InputPinND  = TypedPinNoDefault<PinType::IVec2, InputPin>;
    using IVec3InputPinND  = TypedPinNoDefault<PinType::IVec3, InputPin>;
    using IVec4InputPinND  = TypedPinNoDefault<PinType::IVec4, InputPin>;
    using UVec2InputPinND  = TypedPinNoDefault<PinType::UVec2, InputPin>;
    using UVec3InputPinND  = TypedPinNoDefault<PinType::UVec3, InputPin>;
    using UVec4InputPinND  = TypedPinNoDefault<PinType::UVec4, InputPin>;
    using Vec2InputPinND   = TypedPinNoDefault<PinType::Vec2, InputPin>;
    using Vec3InputPinND   = TypedPinNoDefault<PinType::Vec3, InputPin>;
    using Color3InputPinND = TypedPinNoDefault<PinType::Color3, InputPin>;
    using Vec4InputPinND   = TypedPinNoDefault<PinType::Vec4, InputPin>;
    using Color4InputPinND = TypedPinNoDefault<PinType::Color4, InputPin>;

    using BoolInputPin   = TypedPin<PinType::Bool, bool, InputPin>;
    using IntInputPin    = TypedPin<PinType::Int, int_t, InputPin>;
    using UIntInputPin   = TypedPin<PinType::UInt, uint_t, InputPin>;
    using FloatInputPin  = TypedPin<PinType::Float, float, InputPin>;
    using BVec2InputPin  = TypedPin<PinType::BVec2, BoolVector2D, InputPin>;
    using BVec3InputPin  = TypedPin<PinType::BVec3, BoolVector3D, InputPin>;
    using BVec4InputPin  = TypedPin<PinType::BVec4, BoolVector4D, InputPin>;
    using IVec2InputPin  = TypedPin<PinType::IVec2, IntVector2D, InputPin>;
    using IVec3InputPin  = TypedPin<PinType::IVec3, IntVector3D, InputPin>;
    using IVec4InputPin  = TypedPin<PinType::IVec4, IntVector4D, InputPin>;
    using UVec2InputPin  = TypedPin<PinType::UVec2, UIntVector2D, InputPin>;
    using UVec3InputPin  = TypedPin<PinType::UVec3, UIntVector3D, InputPin>;
    using UVec4InputPin  = TypedPin<PinType::UVec4, UIntVector4D, InputPin>;
    using Vec2InputPin   = TypedPin<PinType::Vec2, Vector2D, InputPin>;
    using Vec3InputPin   = TypedPin<PinType::Vec3, Vector3D, InputPin>;
    using Color3InputPin = TypedPin<PinType::Color3, Vector3D, InputPin>;
    using Vec4InputPin   = TypedPin<PinType::Vec4, Vector4D, InputPin>;
    using Color4InputPin = TypedPin<PinType::Color4, Vector4D, InputPin>;


    using BoolOutputPinND   = TypedPinNoDefault<PinType::Bool, OutputPin>;
    using IntOutputPinND    = TypedPinNoDefault<PinType::Int, OutputPin>;
    using UIntOutputPinND   = TypedPinNoDefault<PinType::UInt, OutputPin>;
    using FloatOutputPinND  = TypedPinNoDefault<PinType::Float, OutputPin>;
    using BVec2OutputPinND  = TypedPinNoDefault<PinType::BVec2, OutputPin>;
    using BVec3OutputPinND  = TypedPinNoDefault<PinType::BVec3, OutputPin>;
    using BVec4OutputPinND  = TypedPinNoDefault<PinType::BVec4, OutputPin>;
    using IVec2OutputPinND  = TypedPinNoDefault<PinType::IVec2, OutputPin>;
    using IVec3OutputPinND  = TypedPinNoDefault<PinType::IVec3, OutputPin>;
    using IVec4OutputPinND  = TypedPinNoDefault<PinType::IVec4, OutputPin>;
    using UVec2OutputPinND  = TypedPinNoDefault<PinType::UVec2, OutputPin>;
    using UVec3OutputPinND  = TypedPinNoDefault<PinType::UVec3, OutputPin>;
    using UVec4OutputPinND  = TypedPinNoDefault<PinType::UVec4, OutputPin>;
    using Vec2OutputPinND   = TypedPinNoDefault<PinType::Vec2, OutputPin>;
    using Vec3OutputPinND   = TypedPinNoDefault<PinType::Vec3, OutputPin>;
    using Color3OutputPinND = TypedPinNoDefault<PinType::Color3, OutputPin>;
    using Vec4OutputPinND   = TypedPinNoDefault<PinType::Vec4, OutputPin>;
    using Color4OutputPinND = TypedPinNoDefault<PinType::Color4, OutputPin>;

    using BoolOutputPin   = TypedPin<PinType::Bool, bool, OutputPin>;
    using IntOutputPin    = TypedPin<PinType::Int, int_t, OutputPin>;
    using UIntOutputPin   = TypedPin<PinType::UInt, uint_t, OutputPin>;
    using FloatOutputPin  = TypedPin<PinType::Float, float, OutputPin>;
    using BVec2OutputPin  = TypedPin<PinType::BVec2, BoolVector2D, OutputPin>;
    using BVec3OutputPin  = TypedPin<PinType::BVec3, BoolVector3D, OutputPin>;
    using BVec4OutputPin  = TypedPin<PinType::BVec4, BoolVector4D, OutputPin>;
    using IVec2OutputPin  = TypedPin<PinType::IVec2, IntVector2D, OutputPin>;
    using IVec3OutputPin  = TypedPin<PinType::IVec3, IntVector3D, OutputPin>;
    using IVec4OutputPin  = TypedPin<PinType::IVec4, IntVector4D, OutputPin>;
    using UVec2OutputPin  = TypedPin<PinType::UVec2, UIntVector2D, OutputPin>;
    using UVec3OutputPin  = TypedPin<PinType::UVec3, UIntVector3D, OutputPin>;
    using UVec4OutputPin  = TypedPin<PinType::UVec4, UIntVector4D, OutputPin>;
    using Vec2OutputPin   = TypedPin<PinType::Vec2, Vector2D, OutputPin>;
    using Vec3OutputPin   = TypedPin<PinType::Vec3, Vector3D, OutputPin>;
    using Color3OutputPin = TypedPin<PinType::Color3, Vector3D, OutputPin>;
    using Vec4OutputPin   = TypedPin<PinType::Vec4, Vector4D, OutputPin>;
    using Color4OutputPin = TypedPin<PinType::Color4, Vector4D, OutputPin>;


    struct ENGINE_EXPORT CompilerState {
    private:
        Map<Identifier, Expression> m_internal_cache;

    public:
        Vector<String> locals;
        Map<Identifier, Any> cache;

        Expression create_variable(const Expression& in_expression);
        Expression expression_cast(const Expression& in_expression, PinType out_type);
        String create_header(const char* prefix) const;

        Expression pin_source(OutputPin* pin);
        Expression pin_source(InputPin* pin);
    };


    class ENGINE_EXPORT Node : public Object
    {
        declare_class(Node, Object);

    protected:
        Vector<InputPin*> m_inputs;
        Vector<OutputPin*> m_outputs;

    public:
        Vector2D position;

        FORCE_INLINE Identifier id() const
        {
            return reinterpret_cast<Identifier>(this);
        }

        virtual bool is_destroyable() const;
        virtual Vector4D header_color() const;

        virtual Expression compile(OutputPin* pin, CompilerState& state);
        virtual Expression compile(InputPin* pin, CompilerState& state);

        const Vector<InputPin*>& inputs() const;
        const Vector<OutputPin*>& outputs() const;
        virtual ~Node();
        virtual const char* name() const;
    };


    ENGINE_EXPORT const char* slang_type_name(PinType type);
    ENGINE_EXPORT String create_default_value(PinType type, const void* data);


#define declare_visual_material_node(NodeName)                                                                                   \
    declare_class(NodeName, Node);                                                                                               \
                                                                                                                                 \
public:                                                                                                                          \
    const char* name() const override;


#define implement_visual_material_node(NodeName, GroupName)                                                                      \
    const char* NodeName::name() const                                                                                           \
    {                                                                                                                            \
        return #NodeName;                                                                                                        \
    }                                                                                                                            \
    implement_class(Engine::VisualMaterialGraph, NodeName, 0)                                                                    \
    {                                                                                                                            \
        if constexpr (!std::is_same_v<Root, NodeName>)                                                                           \
        {                                                                                                                        \
            static_class_instance()->group(Group::find("Engine::VisualMaterialGraphGroups::" #GroupName, true));                 \
        }                                                                                                                        \
    }

    class Root : public Node
    {
        declare_visual_material_node(Root);

        Root();
        Expression compile(InputPin* pin, CompilerState& state) override;
        bool is_destroyable() const override;
    };


    // CONSTANT NODES
#define declare_visual_material_constant_node(type)                                                                              \
    struct type : public Node {                                                                                                  \
        declare_visual_material_node(type);                                                                                      \
                                                                                                                                 \
        type();                                                                                                                  \
        Expression compile(OutputPin* pin, CompilerState& state) override;                                                       \
    };

    declare_visual_material_constant_node(Bool);
    declare_visual_material_constant_node(Int);
    declare_visual_material_constant_node(UInt);
    declare_visual_material_constant_node(Float);
    declare_visual_material_constant_node(BVec2);
    declare_visual_material_constant_node(BVec3);
    declare_visual_material_constant_node(BVec4);
    declare_visual_material_constant_node(IVec2);
    declare_visual_material_constant_node(IVec3);
    declare_visual_material_constant_node(IVec4);
    declare_visual_material_constant_node(UVec2);
    declare_visual_material_constant_node(UVec3);
    declare_visual_material_constant_node(UVec4);
    declare_visual_material_constant_node(Vec2);
    declare_visual_material_constant_node(Vec3);
    declare_visual_material_constant_node(Vec4);
    declare_visual_material_constant_node(Color3);
    declare_visual_material_constant_node(Color4);
#undef declare_visual_material_constant_node
}// namespace Engine::VisualMaterialGraph
